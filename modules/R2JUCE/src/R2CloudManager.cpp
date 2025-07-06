#include "R2CloudManager.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2OneDriveProvider.h"
#include "R2CloudAuthComponent.h"

namespace r2juce {

R2CloudManager::R2CloudManager()
{
    initializeProviders();
    refreshStateAndNotify();
}

R2CloudManager::~R2CloudManager() { onStateChanged = nullptr; }

void R2CloudManager::initializeProviders()
{
    localProvider = createProvider(ServiceType::Local);
    googleDriveProvider = createProvider(ServiceType::GoogleDrive);
    oneDriveProvider = createProvider(ServiceType::OneDrive);
}

const R2CloudManager::AppState& R2CloudManager::getInitialState() const
{
    return currentState;
}

const R2CloudManager::AppState& R2CloudManager::getCurrentState() const
{
    return currentState;
}

std::shared_ptr<R2CloudStorageProvider> R2CloudManager::createProvider(ServiceType type)
{
    switch (type)
    {
        case ServiceType::GoogleDrive: return std::make_shared<R2GoogleDriveProvider>();
        case ServiceType::OneDrive: return std::make_shared<R2OneDriveProvider>();
        case ServiceType::Local:
        default: return std::make_shared<R2LocalStorageProvider>();
    }
}

void R2CloudManager::setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    googleClientId = clientId;
    googleClientSecret = clientSecret;
    if (auto* p = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get())) p->setClientCredentials(clientId, clientSecret);
}

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    oneDriveClientId = clientId;
    oneDriveClientSecret = clientSecret;
    if (auto* p = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get())) p->setClientCredentials(clientId, clientSecret);
}

std::shared_ptr<R2CloudStorageProvider> R2CloudManager::getCurrentProvider()
{
    switch (currentState.selectedService)
    {
        case ServiceType::GoogleDrive: return googleDriveProvider;
        case ServiceType::OneDrive: return oneDriveProvider;
        case ServiceType::Local:
        default: return localProvider;
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
    if (!provider || dynamic_cast<R2LocalStorageProvider*>(provider.get()))
    {
        currentState.authStatus = AuthStatus::Authenticated;
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
        if (auto* p = getCurrentProvider().get()) {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(p))
                googleProv->setTokens(accessToken, refreshToken);
            else if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(p))
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
            case ServiceType::GoogleDrive:
            case ServiceType::OneDrive:
                currentState.isSignOutButtonEnabled = (currentState.authStatus == AuthStatus::Authenticated);
                currentState.areFileButtonsEnabled = (currentState.authStatus == AuthStatus::Authenticated);
                
                juce::String serviceName = (currentState.selectedService == ServiceType::GoogleDrive) ? "Google Drive" : "OneDrive";
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
}

