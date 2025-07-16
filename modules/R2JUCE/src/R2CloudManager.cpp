#include "R2CloudManager.h"
#include "R2CloudAuthComponent.h"
#include "R2GoogleDriveProvider.h"
#include "R2LocalStorageProvider.h"
#include "R2OneDriveProvider.h"

#if JUCE_MAC || JUCE_IOS
#include "R2IcloudDriveProvider.h"
#endif

namespace r2juce {

R2CloudManager::R2CloudManager() {
    // Constructor
}

R2CloudManager::~R2CloudManager() { onStateChanged = nullptr; }

void R2CloudManager::setAppGroupId(const juce::String& groupId) {
    appGroupId = groupId;
}

void R2CloudManager::initializeProviders() {
    localProvider = std::make_shared<R2LocalStorageProvider>(appGroupId);
    
    auto setupProvider = [this](std::shared_ptr<R2CloudStorageProvider>& provider) {
        if (provider)
        {
            provider->onSyncStatusChanged = [this](R2CloudStorageProvider::SyncStatus status, const juce::String& message) {
                this->setSyncStatus(static_cast<R2CloudManager::SyncStatus>(status), message);
            };
        }
    };

    if (googleClientId.isNotEmpty())
    {
        googleDriveProvider = std::make_shared<R2GoogleDriveProvider>(googleClientId, googleClientSecret);
        setupProvider(googleDriveProvider);
    }
    if (oneDriveClientId.isNotEmpty())
    {
        oneDriveProvider = std::make_shared<R2OneDriveProvider>(oneDriveClientId, oneDriveClientSecret);
        setupProvider(oneDriveProvider);
    }

#if JUCE_MAC || JUCE_IOS
    if (iCloudContainerId.isNotEmpty())
        iCloudDriveProvider = std::make_shared<R2IcloudDriveProvider>(iCloudContainerId);
#endif

    refreshStateAndNotify();
}

void R2CloudManager::setCachingForService(ServiceType service, bool enable, const juce::File& cacheDirectory)
{
    DBG("R2CloudManager::setCachingForService called for service " + juce::String((int)service) + " with enable: " + (enable ? "true" : "false"));
    std::shared_ptr<R2CloudStorageProvider> provider;
    std::shared_ptr<R2LocalStorageProvider>* cacheForService = nullptr;

    switch (service)
    {
        case ServiceType::GoogleDrive:
            provider = googleDriveProvider;
            cacheForService = &googleCacheProvider;
            break;
        case ServiceType::OneDrive:
            provider = oneDriveProvider;
            cacheForService = &oneDriveCacheProvider;
            break;
        default:
            return; // Caching is only for remote services
    }

    if (provider && cacheForService)
    {
        if (enable && cacheDirectory.isDirectory())
        {
            auto serviceCacheDir = cacheDirectory.getChildFile(provider->getDisplayName());
            if (!serviceCacheDir.exists())
                serviceCacheDir.createDirectory();

            if (*cacheForService == nullptr)
                *cacheForService = std::make_shared<R2LocalStorageProvider>("");

            (*cacheForService)->setCacheRoot(serviceCacheDir);
            provider->configureCaching(*cacheForService);
        }
        else
        {
            provider->configureCaching(nullptr);
        }
    }
}


const R2CloudManager::AppState& R2CloudManager::getInitialState() const {
    return currentState;
}

const R2CloudManager::AppState& R2CloudManager::getCurrentState() const {
    return currentState;
}

void R2CloudManager::setGoogleCredentials(const juce::String& clientId,
                                          const juce::String& clientSecret) {
    googleClientId = clientId;
    googleClientSecret = clientSecret;
}

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId,
                                            const juce::String& clientSecret) {
    oneDriveClientId = clientId;
    oneDriveClientSecret = clientSecret;
}

#if JUCE_MAC || JUCE_IOS
void R2CloudManager::setIcloudContainerId(const juce::String& containerId) {
    iCloudContainerId = containerId;
}
#endif

std::shared_ptr<R2CloudStorageProvider> R2CloudManager::getProvider() {
    switch (currentState.selectedService) {
        case ServiceType::GoogleDrive:
            return googleDriveProvider;
        case ServiceType::OneDrive:
            return oneDriveProvider;
#if JUCE_MAC || JUCE_IOS
        case ServiceType::iCloudDrive:
            return iCloudDriveProvider;
#endif
        case ServiceType::Local:
        default:
            return localProvider;
    }
}

void R2CloudManager::userSelectedService(ServiceType serviceType) {
    if (currentState.selectedService == serviceType &&
        currentState.authStatus != AuthStatus::Error)
        return;

    previousServiceBeforeAuth = currentState.selectedService;
    currentState.selectedService = serviceType;

    auto provider = getProvider();
    if (!provider) {
        currentState.authStatus = AuthStatus::Error;
        refreshStateAndNotify();
        return;
    }

    auto providerStatus = provider->getAuthStatus();
    if (providerStatus == R2CloudStorageProvider::Status::Authenticated) {
        currentState.authStatus = AuthStatus::Authenticated;
        refreshStateAndNotify();
    } else {
        startAuthenticationFlow();
    }
}

void R2CloudManager::startAuthenticationFlow() {
    auto provider = getProvider();
    if (!provider) {
        currentState.authStatus = AuthStatus::Error;
        refreshStateAndNotify();
        return;
    }

    auto serviceType = provider->getServiceType();
    if (serviceType == ServiceType::Local || serviceType == ServiceType::iCloudDrive) {
        currentState.authStatus =
            (provider->getAuthStatus() == R2CloudStorageProvider::Status::Authenticated)
                ? AuthStatus::Authenticated
                : AuthStatus::Error;
        refreshStateAndNotify();
        return;
    }

    currentState.authStatus = AuthStatus::Authenticating;
    refreshStateAndNotify();

    provider->authenticate([this](bool success, juce::String errorMessage) {
        if (success) {
            juce::MessageManager::callAsync([this]() {
                this->currentState.authStatus = AuthStatus::Authenticated;
                this->refreshStateAndNotify();
            });
        } else {
            if (errorMessage.contains("device flow")) {
                juce::MessageManager::callAsync([this]() {
                    this->currentState.needsAuthUi = true;
                    this->refreshStateAndNotify();
                });
            } else {
                juce::MessageManager::callAsync([this]() {
                    this->currentState.authStatus = AuthStatus::Error;
                    this->refreshStateAndNotify();
                });
            }
        }
    });
}

void R2CloudManager::authenticationFinished(bool success, const juce::String&,
                                            const juce::String& accessToken,
                                            const juce::String& refreshToken) {
    currentState.needsAuthUi = false;
    if (success) {
        if (auto provider = getProvider()) {
            if (auto* googleProv =
                    dynamic_cast<R2GoogleDriveProvider*>(provider.get()))
                googleProv->setTokens(accessToken, refreshToken);
            else if (auto* oneDriveProv =
                         dynamic_cast<R2OneDriveProvider*>(provider.get()))
                oneDriveProv->setTokens(accessToken, refreshToken);
        }
        currentState.authStatus = AuthStatus::Authenticated;
    } else {
        currentState.authStatus = AuthStatus::Error;
        currentState.selectedService = previousServiceBeforeAuth;
    }
    refreshStateAndNotify();
}

void R2CloudManager::userCancelledAuthentication() {
    currentState.needsAuthUi = false;
    currentState.selectedService = previousServiceBeforeAuth;
    currentState.authStatus = AuthStatus::NotAuthenticated;
    refreshStateAndNotify();
}

void R2CloudManager::userSignedOut() {
    if (auto provider = getProvider()) {
        provider->signOut();
    }
    currentState.selectedService = ServiceType::Local;
    currentState.authStatus = AuthStatus::Authenticated;
    refreshStateAndNotify();
}

void R2CloudManager::setSyncStatus(SyncStatus newStatus, const juce::String& message)
{
    juce::ignoreUnused(message);
    currentState.syncStatus = newStatus;
    refreshStateAndNotify();
}

void R2CloudManager::refreshStateAndNotify() {
    auto provider = getProvider();
    if (!provider) {
        currentState.authStatus = AuthStatus::Error;
        currentState.statusLabelText = "Error: Provider not found";
        currentState.isSignOutButtonEnabled = false;
        currentState.areFileButtonsEnabled = false;
    } else {
        if (currentState.authStatus != AuthStatus::Authenticating &&
            !currentState.needsAuthUi) {
            auto providerStatus = provider->getAuthStatus();
            if (providerStatus == R2CloudStorageProvider::Status::Authenticated)
                currentState.authStatus = AuthStatus::Authenticated;
            else if (providerStatus == R2CloudStorageProvider::Status::Error)
                currentState.authStatus = AuthStatus::Error;
            else
                currentState.authStatus = AuthStatus::NotAuthenticated;
        }

        juce::String baseStatusText;
        switch (currentState.selectedService) {
            case ServiceType::Local:
                baseStatusText = "Local Storage";
                currentState.isSignOutButtonEnabled = false;
                currentState.areFileButtonsEnabled = true;
                break;
#if JUCE_MAC || JUCE_IOS
            case ServiceType::iCloudDrive:
                if (iCloudDriveProvider &&
                    iCloudDriveProvider->getAuthStatus() ==
                        R2CloudStorageProvider::Status::Authenticated) {
                    baseStatusText = "iCloud Drive (Available)";
                    currentState.areFileButtonsEnabled = true;
                } else {
                    baseStatusText = "iCloud Drive (Not Available)";
                    currentState.areFileButtonsEnabled = false;
                }
                currentState.isSignOutButtonEnabled = false;
                break;
#endif
            case ServiceType::GoogleDrive:
            case ServiceType::OneDrive:
                currentState.isSignOutButtonEnabled =
                    (currentState.authStatus == AuthStatus::Authenticated);
                currentState.areFileButtonsEnabled =
                    (currentState.authStatus == AuthStatus::Authenticated);

                juce::String serviceName = provider->getDisplayName();
                switch (currentState.authStatus) {
                    case AuthStatus::Authenticated:
                        baseStatusText = serviceName + " (Authenticated)";
                        break;
                    case AuthStatus::NotAuthenticated:
                        baseStatusText = serviceName + " (Not Authenticated)";
                        break;
                    case AuthStatus::Authenticating:
                        baseStatusText = serviceName + " (Authenticating...)";
                        break;
                    case AuthStatus::Error:
                        baseStatusText =
                            serviceName + " (Authentication Error)";
                        break;
                }
                break;
        }

        // Append sync status if relevant
        if (provider->isCachingEnabled())
        {
            switch (currentState.syncStatus)
            {
                case SyncStatus::Syncing:
                    currentState.statusLabelText = baseStatusText + " - Syncing...";
                    break;
                case SyncStatus::Synced:
                    currentState.statusLabelText = baseStatusText + " - Synced";
                    break;
                case SyncStatus::SyncError:
                    currentState.statusLabelText = baseStatusText + " - Sync Error";
                    break;
                case SyncStatus::Idle:
                default:
                    currentState.statusLabelText = baseStatusText;
                    break;
            }
        }
        else
        {
            currentState.statusLabelText = baseStatusText;
        }
    }

    currentState.isComboBoxEnabled =
        (currentState.authStatus != AuthStatus::Authenticating &&
         !currentState.needsAuthUi);

    if (onStateChanged) {
        onStateChanged(currentState);
    }
}

void R2CloudManager::saveFile(const juce::String& filePath,
                              const juce::MemoryBlock& data,
                              FileOperationCallback callback) {
    auto provider = getProvider();
    if (!provider) {
        if (callback) callback(false, "No provider selected");
        return;
    }

    if (currentState.authStatus != AuthStatus::Authenticated) {
        if (callback) callback(false, "Authentication required");
        return;
    }
    provider->uploadFileByPath(filePath, data, callback);
}

bool R2CloudManager::saveFileSync(const juce::String& filePath, const juce::MemoryBlock& data)
{
    auto provider = getProvider();
    if (!provider)
    {
        return false;
    }

    if (currentState.authStatus != AuthStatus::Authenticated)
    {
        return false;
    }
    
    return provider->uploadFileByPathSync(filePath, data);
}

void R2CloudManager::loadFile(const juce::String& filePath,
                              FileContentCallback callback) {
    auto provider = getProvider();
    if (!provider) {
        if (callback) callback(false, "", "No provider selected");
        return;
    }

    if (currentState.authStatus != AuthStatus::Authenticated) {
        if (callback) callback(false, "", "Authentication required");
        return;
    }

    provider->downloadFileByPath(
        filePath, [callback](bool success, juce::MemoryBlock data,
                             juce::String errorMessage) {
            if (success) {
                if (callback)
                    callback(true,
                             juce::String::createStringFromData(data.getData(),
                                                                (int)data.getSize()),
                             "");
            } else {
                if (callback) callback(false, "", errorMessage);
            }
        });
}

}  // namespace r2juce

