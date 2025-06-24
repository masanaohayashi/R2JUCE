#include "R2CloudManager.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2OneDriveProvider.h"
#include "R2CloudAuthComponent.h"

namespace r2juce {

R2CloudManager::R2CloudManager() { initializeProviders(); }
R2CloudManager::~R2CloudManager() { onAuthStatusChanged = nullptr; onServiceChanged = nullptr; }

void R2CloudManager::initializeProviders()
{
    localProvider = createProvider(ServiceType::Local);
    googleDriveProvider = createProvider(ServiceType::GoogleDrive);
    oneDriveProvider = createProvider(ServiceType::OneDrive);
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

void R2CloudManager::selectService(ServiceType serviceType)
{
    if (currentServiceType == serviceType) return;
    hideAuthenticationUI();
    currentServiceType = serviceType;
    updateAuthStatus();
    if (onServiceChanged) onServiceChanged(serviceType);
}

R2CloudManager::AuthStatus R2CloudManager::getAuthStatus() const { return currentAuthStatus; }

std::shared_ptr<R2CloudStorageProvider> R2CloudManager::getCurrentProvider()
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive: return googleDriveProvider;
        case ServiceType::OneDrive: return oneDriveProvider;
        case ServiceType::Local: default: return localProvider;
    }
}

std::shared_ptr<const R2CloudStorageProvider> R2CloudManager::getCurrentProvider() const
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive: return googleDriveProvider;
        case ServiceType::OneDrive: return oneDriveProvider;
        case ServiceType::Local: default: return localProvider;
    }
}

void R2CloudManager::updateAuthStatus()
{
    auto provider = getCurrentProvider();
    if (!provider) { setAuthStatus(AuthStatus::Error); return; }
    
    AuthStatus newStatus;
    switch (provider->getAuthStatus())
    {
        case R2CloudStorageProvider::Status::Authenticated: newStatus = AuthStatus::Authenticated; break;
        case R2CloudStorageProvider::Status::Authenticating: newStatus = AuthStatus::Authenticating; break;
        case R2CloudStorageProvider::Status::Error: newStatus = AuthStatus::Error; break;
        default: newStatus = AuthStatus::NotAuthenticated; break;
    }
    setAuthStatus(newStatus);
}

void R2CloudManager::setAuthStatus(AuthStatus newStatus)
{
    if (currentAuthStatus != newStatus)
    {
        currentAuthStatus = newStatus;
        if (onAuthStatusChanged) onAuthStatusChanged(newStatus);
    }
}

bool R2CloudManager::needsAuthentication() const
{
    auto provider = getCurrentProvider();
    return provider ? (provider->getAuthStatus() != R2CloudStorageProvider::Status::Authenticated) : false;
}

void R2CloudManager::showAuthenticationUI(juce::Component* parent)
{
    auto provider = getCurrentProvider();
    if (!provider) { return; }

    if (dynamic_cast<R2LocalStorageProvider*>(provider.get()))
    {
        setAuthStatus(AuthStatus::Authenticated);
        return;
    }

    setAuthStatus(AuthStatus::Authenticating);
    
    provider->authenticate([this, parent](bool success, juce::String errorMessage)
    {
        if (success)
        {
            juce::MessageManager::callAsync([this]() { setAuthStatus(AuthStatus::Authenticated); });
        }
        else
        {
            if (errorMessage.contains("device flow"))
            {
                juce::MessageManager::callAsync([this, parent]() { startDeviceFlowAuthentication(parent); });
            }
            else
            {
                juce::MessageManager::callAsync([this]() { setAuthStatus(AuthStatus::Error); });
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

    authComponent->onAuthenticationCancelled = [this]() {
        hideAuthenticationUI();
        setAuthStatus(AuthStatus::NotAuthenticated);
    };

    if (parent)
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
        if (parentForAuth) parentForAuth->removeChildComponent(authComponent.get());
        authComponent.reset();
        parentForAuth = nullptr;
    }
}

void R2CloudManager::handleAuthenticationComplete(bool success, const juce::String&, const juce::String& accessToken, const juce::String& refreshToken)
{
    hideAuthenticationUI();
    if (success)
    {
        if (auto* p = getCurrentProvider().get()) {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(p))
                googleProv->setTokens(accessToken, refreshToken);
            else if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(p))
                oneDriveProv->setTokens(accessToken, refreshToken);
        }
        setAuthStatus(AuthStatus::Authenticated);
    } else {
        setAuthStatus(AuthStatus::Error);
    }
}

void R2CloudManager::signOut()
{
    if (auto provider = getCurrentProvider()) provider->signOut();
    updateAuthStatus();
    hideAuthenticationUI();
}

// Modified: contentをjuce::MemoryBlockで受け取る
void R2CloudManager::saveFile(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) { if (callback) callback(false, "No provider"); return; }
    if (needsAuthentication()) { if (callback) callback(false, "Authentication required"); return; }
    provider->uploadFileByPath(filePath, data, callback); // MemoryBlockを直接渡す
}

void R2CloudManager::loadFile(const juce::String& filePath, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) { if (callback) callback(false, "", "No provider"); return; }
    if (needsAuthentication()) { if (callback) callback(false, "", "Authentication required"); return; }
    provider->downloadFileByPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String errorMessage)
    {
        if (success) { if (callback) callback(true, juce::String::createStringFromData(data.getData(), (int)data.getSize()), ""); }
        else { if (callback) callback(false, "", errorMessage); }
    });
}
}
