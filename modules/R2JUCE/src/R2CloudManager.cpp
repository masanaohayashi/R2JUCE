#include "R2CloudManager.h"
#include "R2CloudAuthComponent.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2OneDriveProvider.h"

#if JUCE_MAC || JUCE_IOS
#include "R2IcloudDriveProvider.h"
#endif

namespace r2juce {

R2CloudManager::R2CloudManager()
{
    // Constructor
}

R2CloudManager::~R2CloudManager()
{
    onStateChanged = nullptr;
}

void R2CloudManager::initializeProviders()
{
    localProvider = std::make_shared<R2LocalStorageProvider>();
    googleDriveProvider = std::make_shared<R2GoogleDriveProvider>();
    oneDriveProvider = std::make_shared<R2OneDriveProvider>();
#if JUCE_MAC || JUCE_IOS
    // Create with default constructor
    iCloudDriveProvider = std::make_shared<R2IcloudDriveProvider>();
    // Set the container ID if it was provided before initialization
    if (iCloudContainerId.isNotEmpty())
        if (auto* p = dynamic_cast<R2IcloudDriveProvider*>(iCloudDriveProvider.get()))
            p->setContainerId(iCloudContainerId);
#endif

    if (googleClientId.isNotEmpty())
        if (auto* p = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
            p->setClientCredentials(googleClientId, googleClientSecret);
            
    if (oneDriveClientId.isNotEmpty())
        if (auto* p = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
            p->setClientCredentials(oneDriveClientId, oneDriveClientSecret);

    refreshStateAndNotify();
}

const R2CloudManager::AppState& R2CloudManager::getInitialState() const
{
    return currentState;
}

const R2CloudManager::AppState& R2CloudManager::getCurrentState() const
{
    return currentState;
}

void R2CloudManager::setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    googleClientId = clientId;
    googleClientSecret = clientSecret;
    if (googleDriveProvider)
        if (auto* p = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
            p->setClientCredentials(clientId, clientSecret);
}

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    oneDriveClientId = clientId;
    oneDriveClientSecret = clientSecret;
    if (oneDriveProvider)
        if (auto* p = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
            p->setClientCredentials(clientId, clientSecret);
}

#if JUCE_MAC || JUCE_IOS
void R2CloudManager::setIcloudContainerId(const juce::String& containerId)
{
    iCloudContainerId = containerId;
    // If the provider already exists, configure it.
    if (iCloudDriveProvider)
        if (auto* p = dynamic_cast<R2IcloudDriveProvider*>(iCloudDriveProvider.get()))
            p->setContainerId(containerId);
}
#endif

std::shared_ptr<R2CloudStorageProvider> R2CloudManager::getCurrentProvider()
{
    switch (currentState.selectedService)
    {
        case ServiceType::GoogleDrive: return googleDriveProvider;
        case ServiceType::OneDrive:   return oneDriveProvider;
#if JUCE_MAC || JUCE_IOS
        case ServiceType::iCloudDrive: return iCloudDriveProvider;
#endif
        case ServiceType::Local:
        default:                      return localProvider;
    }
}

void R2CloudManager::userSelectedService(ServiceType serviceType)
{
    if (currentState.selectedService == serviceType && currentState.authStatus != AuthStatus::Error)
        return;

    previousServiceBeforeAuth = currentState.selectedService;
    currentState.selectedService = serviceType;
    
    auto provider = getCurrentProvider();
    if (!provider)
    {
        currentState.authStatus = AuthStatus::Error;
        refreshStateAndNotify();
        return;
    }
    
    auto providerStatus = provider->getAuthStatus();
    if (providerStatus == R2CloudStorageProvider::Status::Authenticated)
    {
        currentState.authStatus = AuthStatus::Authenticated;
        refreshStateAndNotify();
    }
    else
    {
        startAuthenticationFlow();
    }
}

void R2CloudManager::startAuthenticationFlow()
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        currentState.authStatus = AuthStatus::Error;
        refreshStateAndNotify();
        return;
    }

    auto serviceType = provider->getServiceType();
    if (serviceType == ServiceType::Local || serviceType == ServiceType::iCloudDrive)
    {
        currentState.authStatus = (provider->getAuthStatus() == R2CloudStorageProvider::Status::Authenticated)
                                     ? AuthStatus::Authenticated
                                     : AuthStatus::Error;
        refreshStateAndNotify();
        return;
    }

    currentState.authStatus = AuthStatus::Authenticating;
    refreshStateAndNotify();

    provider->authenticate([this](bool success, juce::String errorMessage)
    {
        if (success)
        {
            juce::MessageManager::callAsync([this]() {
                this->currentState.authStatus = AuthStatus::Authenticated;
                this->refreshStateAndNotify();
            });
        }
        else
        {
            if (errorMessage.contains("device flow"))
            {
                juce::MessageManager::callAsync([this]() {
                    this->currentState.needsAuthUi = true;
                    this->refreshStateAndNotify();
                });
            }
            else
            {
                juce::MessageManager::callAsync([this]() {
                    this->currentState.authStatus = AuthStatus::Error;
                    this->refreshStateAndNotify();
                });
            }
        }
    });
}

void R2CloudManager::authenticationFinished(bool success, const juce::String&, const juce::String& accessToken, const juce::String& refreshToken)
{
    currentState.needsAuthUi = false;
    if (success)
    {
        if (auto provider = getCurrentProvider()) {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(provider.get()))
                googleProv->setTokens(accessToken, refreshToken);
            else if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(provider.get()))
                oneDriveProv->setTokens(accessToken, refreshToken);
        }
        currentState.authStatus = AuthStatus::Authenticated;
    } else {
        currentState.authStatus = AuthStatus::Error;
        currentState.selectedService = previousServiceBeforeAuth;
    }
    refreshStateAndNotify();
}

void R2CloudManager::userCancelledAuthentication()
{
    currentState.needsAuthUi = false;
    currentState.selectedService = previousServiceBeforeAuth;
    currentState.authStatus = AuthStatus::NotAuthenticated;
    refreshStateAndNotify();
}

void R2CloudManager::userSignedOut()
{
    if (auto provider = getCurrentProvider())
    {
        provider->signOut();
    }
    currentState.selectedService = ServiceType::Local;
    currentState.authStatus = AuthStatus::Authenticated;
    refreshStateAndNotify();
}

void R2CloudManager::refreshStateAndNotify()
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        currentState.authStatus = AuthStatus::Error;
        currentState.statusLabelText = "Error: Provider not found";
        currentState.isSignOutButtonEnabled = false;
        currentState.areFileButtonsEnabled = false;
    }
    else
    {
        if (currentState.authStatus != AuthStatus::Authenticating && !currentState.needsAuthUi)
        {
            auto providerStatus = provider->getAuthStatus();
            if(providerStatus == R2CloudStorageProvider::Status::Authenticated)
                currentState.authStatus = AuthStatus::Authenticated;
            else if (providerStatus == R2CloudStorageProvider::Status::Error)
                currentState.authStatus = AuthStatus::Error;
            else
                currentState.authStatus = AuthStatus::NotAuthenticated;
        }

        switch (currentState.selectedService)
        {
            case ServiceType::Local:
                currentState.statusLabelText = "Local Storage";
                currentState.isSignOutButtonEnabled = false;
                currentState.areFileButtonsEnabled = true;
                break;
#if JUCE_MAC || JUCE_IOS
            case ServiceType::iCloudDrive:
                if (iCloudDriveProvider && iCloudDriveProvider->getAuthStatus() == R2CloudStorageProvider::Status::Authenticated)
                {
                    currentState.statusLabelText = "iCloud Drive (Available)";
                    currentState.areFileButtonsEnabled = true;
                }
                else
                {
                    currentState.statusLabelText = "iCloud Drive (Not Available)";
                    currentState.areFileButtonsEnabled = false;
                }
                currentState.isSignOutButtonEnabled = false;
                break;
#endif
            case ServiceType::GoogleDrive:
            case ServiceType::OneDrive:
                currentState.isSignOutButtonEnabled = (currentState.authStatus == AuthStatus::Authenticated);
                currentState.areFileButtonsEnabled = (currentState.authStatus == AuthStatus::Authenticated);
                
                juce::String serviceName = provider->getDisplayName();
                switch (currentState.authStatus)
                {
                    case AuthStatus::Authenticated:
                        currentState.statusLabelText = serviceName + " (Authenticated)";
                        break;
                    case AuthStatus::NotAuthenticated:
                        currentState.statusLabelText = serviceName + " (Not Authenticated)";
                        break;
                    case AuthStatus::Authenticating:
                        currentState.statusLabelText = serviceName + " (Authenticating...)";
                        break;
                    case AuthStatus::Error:
                        currentState.statusLabelText = serviceName + " (Authentication Error)";
                        break;
                }
                break;
        }
    }
    
    currentState.isComboBoxEnabled = (currentState.authStatus != AuthStatus::Authenticating && !currentState.needsAuthUi);

    if (onStateChanged)
    {
        onStateChanged(currentState);
    }
}

void R2CloudManager::saveFile(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) { if (callback) callback(false, "No provider selected"); return; }
    
    if (currentState.authStatus != AuthStatus::Authenticated)
    {
        if (callback) callback(false, "Authentication required");
        return;
    }
    provider->uploadFileByPath(filePath, data, callback);
}

void R2CloudManager::loadFile(const juce::String& filePath, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) { if (callback) callback(false, "", "No provider selected"); return; }

    if (currentState.authStatus != AuthStatus::Authenticated)
    {
        if (callback) callback(false, "", "Authentication required");
        return;
    }

    provider->downloadFileByPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String errorMessage)
    {
        if (success) { if (callback) callback(true, juce::String::createStringFromData(data.getData(), (int)data.getSize()), ""); }
        else { if (callback) callback(false, "", errorMessage); }
    });
}

} // namespace r2juce

