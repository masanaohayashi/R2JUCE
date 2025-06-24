#include "R2CloudManager.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2OneDriveProvider.h"
#include "R2CloudAuthComponent.h"

namespace r2juce {

R2CloudManager::R2CloudManager()
{
    initializeProviders();
}

R2CloudManager::~R2CloudManager()
{
    DBG("R2CloudManager destructor called");
    
    cancelAuthentication();
    
    onAuthStatusChanged = nullptr;
    onServiceChanged = nullptr;
    
    juce::Thread::sleep(10);
    
    localProvider.reset();
    googleDriveProvider.reset();
    oneDriveProvider.reset();
}

void R2CloudManager::initializeProviders()
{
    localProvider = createProvider(ServiceType::Local);
    googleDriveProvider = createProvider(ServiceType::GoogleDrive);
    oneDriveProvider = createProvider(ServiceType::OneDrive);
    
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    auto cloudDocDir = documentsDir.getChildFile("CloudDoc");
    setLocalStorageDirectory(cloudDocDir);
}

std::unique_ptr<R2CloudStorageProvider> R2CloudManager::createProvider(ServiceType type)
{
    switch (type)
    {
        case ServiceType::GoogleDrive:
            return std::make_unique<R2GoogleDriveProvider>();
        case ServiceType::OneDrive:
            // return std::make_unique<R2OneDriveProvider>(); // This would be the implementation
            return nullptr; // Placeholder
        case ServiceType::Local:
        default:
            return std::make_unique<R2LocalStorageProvider>();
    }
}

void R2CloudManager::setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    googleClientId = clientId;
    googleClientSecret = clientSecret;
    
    if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
    {
        googleProv->setClientCredentials(clientId, clientSecret);
    }
}

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    oneDriveClientId = clientId;
    oneDriveClientSecret = clientSecret;
    
    /*
    if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
    {
        oneDriveProv->setClientCredentials(clientId, clientSecret);
    }
    */
}

void R2CloudManager::setLocalStorageDirectory(const juce::File& directory)
{
    localStorageDir = directory;
    
    if (auto* localProv = dynamic_cast<R2LocalStorageProvider*>(localProvider.get()))
    {
        localProv->setRootDirectory(directory);
    }
}

void R2CloudManager::selectService(ServiceType serviceType)
{
    if (currentServiceType == serviceType)
        return;
    
    hideAuthenticationUI();
    
    currentServiceType = serviceType;
    updateAuthStatus();
    
    if (onServiceChanged)
        onServiceChanged(serviceType);
}

R2CloudManager::AuthStatus R2CloudManager::getAuthStatus() const
{
    return currentAuthStatus;
}

R2CloudStorageProvider* R2CloudManager::getCurrentProvider()
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive:
            return googleDriveProvider.get();
        case ServiceType::OneDrive:
            return oneDriveProvider.get();
        case ServiceType::Local:
        default:
            return localProvider.get();
    }
}

const R2CloudStorageProvider* R2CloudManager::getCurrentProvider() const
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive:
            return googleDriveProvider.get();
        case ServiceType::OneDrive:
            return oneDriveProvider.get();
        case ServiceType::Local:
        default:
            return localProvider.get();
    }
}

void R2CloudManager::updateAuthStatus()
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        setAuthStatus(AuthStatus::Error);
        return;
    }
    
    AuthStatus newStatus;
    switch (provider->getAuthStatus())
    {
        case R2CloudStorageProvider::Status::Authenticated:
            newStatus = AuthStatus::Authenticated;
            break;
        case R2CloudStorageProvider::Status::Authenticating:
            newStatus = AuthStatus::Authenticating;
            break;
        case R2CloudStorageProvider::Status::Error:
            newStatus = AuthStatus::Error;
            break;
        case R2CloudStorageProvider::Status::NotAuthenticated:
        default:
            newStatus = AuthStatus::NotAuthenticated;
            break;
    }
    
    setAuthStatus(newStatus);
}

void R2CloudManager::setAuthStatus(AuthStatus newStatus)
{
    if (currentAuthStatus != newStatus)
    {
        currentAuthStatus = newStatus;
        if (onAuthStatusChanged)
            onAuthStatusChanged(newStatus);
    }
}

bool R2CloudManager::needsAuthentication() const
{
    auto provider = getCurrentProvider();
    if (!provider)
        return false;
    
    return provider->getAuthStatus() != R2CloudStorageProvider::Status::Authenticated;
}

void R2CloudManager::showAuthenticationUI(juce::Component* parent)
{
    auto provider = getCurrentProvider();
    if (provider == nullptr)
    {
        return;
    }

    if (authComponent != nullptr)
    {
        cancelAuthentication();
    }

    if (currentServiceType == ServiceType::Local)
    {
        setAuthStatus(AuthStatus::Authenticated);
        return;
    }

    currentAuthStatus = AuthStatus::Authenticating;
    if (onAuthStatusChanged)
        onAuthStatusChanged(currentAuthStatus);
    
    provider->authenticate([this, parent](bool success, juce::String errorMessage)
    {
        if (success)
        {
            juce::MessageManager::callAsync([this]() {
                setAuthStatus(AuthStatus::Authenticated);
            });
        }
        else
        {
            if (errorMessage.contains("device flow") || errorMessage.contains("authentication required"))
            {
                juce::MessageManager::callAsync([this, parent]() {
                    this->startDeviceFlowAuthentication(parent);
                });
            }
            else
            {
                juce::MessageManager::callAsync([this]() {
                    setAuthStatus(AuthStatus::Error);
                });
            }
        }
    });
}

void R2CloudManager::startDeviceFlowAuthentication(juce::Component* parent)
{
    authComponent = std::make_unique<R2CloudAuthComponent>();
    parentForAuth = parent;

    if (currentServiceType == ServiceType::GoogleDrive)
    {
        authComponent->setServiceType(R2CloudAuthComponent::ServiceType::GoogleDrive);
        authComponent->setGoogleCredentials(googleClientId, googleClientSecret);
    }
    else if (currentServiceType == ServiceType::OneDrive)
    {
        authComponent->setServiceType(R2CloudAuthComponent::ServiceType::OneDrive);
        authComponent->setOneDriveCredentials(oneDriveClientId, oneDriveClientSecret);
    }

    authComponent->onAuthenticationComplete = [this](bool success, const juce::String& errorMessage,
        const juce::String& accessToken, const juce::String& refreshToken)
    {
        handleAuthenticationComplete(success, errorMessage, accessToken, refreshToken);
    };

    authComponent->onAuthenticationCancelled = [this]()
    {
        hideAuthenticationUI();
        setAuthStatus(AuthStatus::NotAuthenticated);
    };

    if (parent != nullptr)
    {
        parent->addAndMakeVisible(*authComponent);
        authComponent->setBounds(parent->getLocalBounds());
    }

    authComponent->startAuthentication();
}

void R2CloudManager::hideAuthenticationUI()
{
    if (authComponent)
    {
        authComponent->stopAuthentication();
        
        if (parentForAuth)
        {
            parentForAuth->removeChildComponent(authComponent.get());
        }
        
        authComponent.reset();
        parentForAuth = nullptr;
    }
}

void R2CloudManager::handleAuthenticationComplete(bool success, const juce::String& errorMessage, const juce::String& accessToken, const juce::String& refreshToken)
{
    hideAuthenticationUI();
    
    if (success)
    {
        if (currentServiceType == ServiceType::GoogleDrive)
        {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
            {
                googleProv->setTokens(accessToken, refreshToken);
            }
        }
        else if (currentServiceType == ServiceType::OneDrive)
        {
            // if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
            // {
            //     oneDriveProv->setTokens(accessToken, refreshToken);
            // }
        }
        setAuthStatus(AuthStatus::Authenticated);
    }
    else
    {
        setAuthStatus(AuthStatus::Error);
    }
    
    updateAuthStatus();
}

void R2CloudManager::signOut()
{
    auto provider = getCurrentProvider();
    if (provider)
    {
        provider->signOut();
        updateAuthStatus();
    }
    
    hideAuthenticationUI();
}

void R2CloudManager::cancelAuthentication()
{
    DBG("R2CloudManager::cancelAuthentication() called");
    
    onAuthStatusChanged = nullptr;
    onServiceChanged = nullptr;
    
    hideAuthenticationUI();
    
    if (auto provider = getCurrentProvider())
    {
        if (auto* googleProvider = dynamic_cast<R2GoogleDriveProvider*>(provider))
        {
            googleProvider->cancelAuthentication();
        }
    }
    
    currentAuthStatus = AuthStatus::NotAuthenticated;
    
    juce::Thread::sleep(50);
}

void R2CloudManager::saveFile(const juce::String& filePath, const juce::String& content, FileOperationCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        if (callback)
            callback(false, "No storage provider available");
        return;
    }
    
    if (needsAuthentication())
    {
        if (callback)
            callback(false, "Authentication required");
        return;
    }
    
    juce::MemoryBlock data(content.toUTF8(), content.getNumBytesAsUTF8());
    
    // Now we call the unified interface method directly. No more branching!
    provider->uploadFileByPath(filePath, data, [callback](bool success, juce::String errorMessage)
    {
        if (callback)
            callback(success, errorMessage);
    });
}

void R2CloudManager::loadFile(const juce::String& filePath, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        if (callback)
            callback(false, "", "No storage provider available");
        return;
    }
    
    if (needsAuthentication())
    {
        if (callback)
            callback(false, "", "Authentication required");
        return;
    }

    // Now we call the unified interface method directly.
    // The complex if/else/dynamic_cast block is gone.
    provider->downloadFileByPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String errorMessage)
    {
        if (success)
        {
            auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
            if (callback)
                callback(true, content, "");
        }
        else
        {
            if (callback)
                callback(false, "", "Failed to download file: " + errorMessage);
        }
    });
}

} // namespace r2juce
