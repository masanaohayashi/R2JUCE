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
    DBG("R2CloudManager::initializeProviders()");
    localProvider = createProvider(ServiceType::Local);
    googleDriveProvider = createProvider(ServiceType::GoogleDrive);
    oneDriveProvider = createProvider(ServiceType::OneDrive);
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    setLocalStorageDirectory(documentsDir.getChildFile("CloudDoc"));
}

std::unique_ptr<R2CloudStorageProvider> R2CloudManager::createProvider(ServiceType type)
{
    switch (type)
    {
        case ServiceType::GoogleDrive: return std::make_unique<R2GoogleDriveProvider>();
        case ServiceType::OneDrive: return std::make_unique<R2OneDriveProvider>();
        case ServiceType::Local:
        default: return std::make_unique<R2LocalStorageProvider>();
    }
}

void R2CloudManager::setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    if (auto* p = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get())) p->setClientCredentials(clientId, clientSecret);
}

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    if (auto* p = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get())) p->setClientCredentials(clientId, clientSecret);
}

void R2CloudManager::setLocalStorageDirectory(const juce::File& directory)
{
    if (auto* p = dynamic_cast<R2LocalStorageProvider*>(localProvider.get())) p->setRootDirectory(directory);
}

void R2CloudManager::selectService(ServiceType serviceType)
{
    DBG("R2CloudManager::selectService() - serviceType: " + juce::String((int)serviceType));
    if (currentServiceType == serviceType) return;
    
    hideAuthenticationUI();
    currentServiceType = serviceType;
    updateAuthStatus();
    if (onServiceChanged) onServiceChanged(serviceType);
}

R2CloudManager::AuthStatus R2CloudManager::getAuthStatus() const { return currentAuthStatus; }

R2CloudStorageProvider* R2CloudManager::getCurrentProvider()
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive: return googleDriveProvider.get();
        case ServiceType::OneDrive: return oneDriveProvider.get();
        case ServiceType::Local: default: return localProvider.get();
    }
}

// === FIX START ===
// This is the missing function definition that caused the linker error.
const R2CloudStorageProvider* R2CloudManager::getCurrentProvider() const
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive: return googleDriveProvider.get();
        case ServiceType::OneDrive: return oneDriveProvider.get();
        case ServiceType::Local: default: return localProvider.get();
    }
}
// === FIX END ===

void R2CloudManager::updateAuthStatus()
{
    auto provider = getCurrentProvider();
    if (!provider) { setAuthStatus(AuthStatus::Error); return; }
    
    DBG("R2CloudManager::updateAuthStatus() - checking provider status...");
    AuthStatus newStatus;
    switch (provider->getAuthStatus())
    {
        case R2CloudStorageProvider::Status::Authenticated: newStatus = AuthStatus::Authenticated; break;
        case R2CloudStorageProvider::Status::Authenticating: newStatus = AuthStatus::Authenticating; break;
        case R2CloudStorageProvider::Status::Error: newStatus = AuthStatus::Error; break;
        default: newStatus = AuthStatus::NotAuthenticated; break;
    }
    DBG("R2CloudManager::updateAuthStatus() - Provider status is " + juce::String((int)provider->getAuthStatus()) + ", Manager status will be " + juce::String((int)newStatus));
    setAuthStatus(newStatus);
}

void R2CloudManager::setAuthStatus(AuthStatus newStatus)
{
    if (currentAuthStatus != newStatus)
    {
        DBG("R2CloudManager::setAuthStatus() - Status changing from " + juce::String((int)currentAuthStatus) + " to " + juce::String((int)newStatus));
        currentAuthStatus = newStatus;
        if (onAuthStatusChanged) onAuthStatusChanged(newStatus);
    }
}

bool R2CloudManager::needsAuthentication() const
{
    auto provider = getCurrentProvider();
    bool needsAuth = provider ? (provider->getAuthStatus() != R2CloudStorageProvider::Status::Authenticated) : false;
    DBG("R2CloudManager::needsAuthentication() - Returning " + juce::String(needsAuth ? "true" : "false"));
    return needsAuth;
}

void R2CloudManager::showAuthenticationUI(juce::Component* parent)
{
    DBG("R2CloudManager::showAuthenticationUI()");
    auto provider = getCurrentProvider();
    if (!provider) { DBG("R2CloudManager::showAuthenticationUI() - No provider, returning."); return; }

    if (currentServiceType == ServiceType::Local)
    {
        DBG("R2CloudManager::showAuthenticationUI() - Service is Local, setting status to Authenticated.");
        setAuthStatus(AuthStatus::Authenticated);
        return;
    }

    DBG("R2CloudManager::showAuthenticationUI() - Calling provider->authenticate()");
    currentAuthStatus = AuthStatus::Authenticating; // Set status before async call
    if (onAuthStatusChanged) onAuthStatusChanged(currentAuthStatus);
    
    provider->authenticate([this, parent](bool success, juce::String errorMessage)
    {
        DBG("R2CloudManager::authenticate callback received - success: " + juce::String(success ? "true" : "false") + ", message: " + errorMessage);
        if (success)
        {
            juce::MessageManager::callAsync([this]() { setAuthStatus(AuthStatus::Authenticated); });
        }
        else
        {
            if (errorMessage.contains("device flow"))
            {
                DBG("R2CloudManager::authenticate callback - 'device flow' detected, starting UI flow.");
                juce::MessageManager::callAsync([this, parent]() { startDeviceFlowAuthentication(parent); });
            }
            else
            {
                DBG("R2CloudManager::authenticate callback - Generic error, setting status to Error.");
                juce::MessageManager::callAsync([this]() { setAuthStatus(AuthStatus::Error); });
            }
        }
    });
}

void R2CloudManager::startDeviceFlowAuthentication(juce::Component* parent)
{
    DBG("R2CloudManager::startDeviceFlowAuthentication()");
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
        DBG("R2CloudManager::hideAuthenticationUI()");
        authComponent->stopAuthentication();
        if (parentForAuth) parentForAuth->removeChildComponent(authComponent.get());
        authComponent.reset();
        parentForAuth = nullptr;
    }
}

void R2CloudManager::handleAuthenticationComplete(bool success, const juce::String& errorMessage, const juce::String& accessToken, const juce::String& refreshToken)
{
    DBG("R2CloudManager::handleAuthenticationComplete() - success: " + juce::String(success ? "true" : "false") + ", errorMessage: " + errorMessage);
    hideAuthenticationUI();
    if (success)
    {
        if (auto* p = getCurrentProvider())
        {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(p)) googleProv->setTokens(accessToken, refreshToken);
            else if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(p)) oneDriveProv->setTokens(accessToken, refreshToken);
        }
        setAuthStatus(AuthStatus::Authenticated);
    }
    else
    {
        setAuthStatus(AuthStatus::Error);
    }
}

void R2CloudManager::signOut()
{
    DBG("R2CloudManager::signOut()");
    if (auto provider = getCurrentProvider()) provider->signOut();
    updateAuthStatus();
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
    if (!provider) { if (callback) callback(false, "No provider"); return; }
    if (needsAuthentication()) { if (callback) callback(false, "Authentication required"); return; }
    
    DBG("R2CloudManager::saveFile() - Calling provider->uploadFileByPath for: " + filePath);
    provider->uploadFileByPath(filePath, juce::MemoryBlock(content.toUTF8(), content.getNumBytesAsUTF8()), [callback](bool success, juce::String message) {
        if (callback)
            callback(success, message);
    });
}

void R2CloudManager::loadFile(const juce::String& filePath, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) { if (callback) callback(false, "", "No provider"); return; }
    if (needsAuthentication()) { if (callback) callback(false, "", "Authentication required"); return; }

    DBG("R2CloudManager::loadFile() - Calling provider->downloadFileByPath for: " + filePath);
    provider->downloadFileByPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String errorMessage)
    {
        if (success)
        {
            if (callback) callback(true, juce::String::createStringFromData(data.getData(), (int)data.getSize()), "");
        }
        else
        {
            if (callback) callback(false, "", errorMessage);
        }
    });
}

} // namespace r2juce
