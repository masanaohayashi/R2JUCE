
#include "R2CloudManager.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2CloudAuthComponent.h"

namespace r2juce {

R2CloudManager::R2CloudManager()
{
    initializeProviders();
}

R2CloudManager::~R2CloudManager()
{
    hideAuthenticationUI();
}

void R2CloudManager::initializeProviders()
{
    localProvider = createProvider(ServiceType::Local);
    googleDriveProvider = createProvider(ServiceType::GoogleDrive);
    
    // デフォルトのローカルストレージディレクトリを設定
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
        case ServiceType::Local:
        default:
            return std::make_unique<R2LocalStorageProvider>();
        // 今後追加予定:
        // case ServiceType::Dropbox: return std::make_unique<R2DropboxProvider>();
        // case ServiceType::OneDrive: return std::make_unique<R2OneDriveProvider>();
        // case ServiceType::iCloud: return std::make_unique<R2iCloudProvider>();
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
    
    // 認証UIを非表示
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
        case ServiceType::Local:
        default:
            return localProvider.get();
        // 今後追加予定:
        // case ServiceType::iCloud: return iCloudProvider.get();
        // case ServiceType::Dropbox: return dropboxProvider.get();
        // case ServiceType::OneDrive: return oneDriveProvider.get();
    }
}

const R2CloudStorageProvider* R2CloudManager::getCurrentProvider() const
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive:
            return googleDriveProvider.get();
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

void R2CloudManager::showAuthenticationUI(juce::Component* parentComponent)
{
    if (currentServiceType == ServiceType::Local)
        return; // ローカルストレージは認証不要
    
    if (!parentComponent)
        return;
    
    hideAuthenticationUI(); // 既存のUIを非表示
    
    parentForAuth = parentComponent;
    authComponent.reset(new R2CloudAuthComponent);
    
    // 認証コンポーネントの設定
    if (currentServiceType == ServiceType::GoogleDrive)
    {
        authComponent->setGoogleCredentials(googleClientId, googleClientSecret);
        authComponent->setServiceType(R2CloudAuthComponent::ServiceType::GoogleDrive);
    }
    
    // コールバック設定
    authComponent->onAuthenticationComplete = [this](bool success, juce::String errorMessage, juce::String accessToken, juce::String refreshToken)
    {
        handleAuthenticationComplete(success, errorMessage, accessToken, refreshToken);
    };

    authComponent->onAuthenticationCancelled = [this]()
    {
        hideAuthenticationUI();
        setAuthStatus(AuthStatus::NotAuthenticated);
    };
    
    // 親コンポーネントに追加（オーバーレイ表示）
    parentForAuth->addAndMakeVisible(*authComponent);
    authComponent->setBounds(parentForAuth->getLocalBounds());
    authComponent->toFront(true);
    
    // 認証開始
    setAuthStatus(AuthStatus::Authenticating);
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
        // ✅ トークンをプロバイダーに設定
        if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
        {
            googleProv->setTokens(accessToken, refreshToken);
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

void R2CloudManager::saveFile(const juce::String& filename, const juce::String& content, FileOperationCallback callback)
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
    
    provider->uploadFile(filename, data, "root", [callback](bool success, juce::String errorMessage)
    {
        if (callback)
            callback(success, errorMessage);
    });
}

void R2CloudManager::loadFile(const juce::String& filename, FileContentCallback callback)
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
    
    // ローカルストレージの場合の特別処理
    if (auto* localProv = dynamic_cast<R2LocalStorageProvider*>(provider))
    {
        auto file = localStorageDir.getChildFile(filename);
        if (file.exists())
        {
            auto content = file.loadFileAsString();
            if (callback)
                callback(true, content, "");
        }
        else
        {
            if (callback)
                callback(false, "", "File not found: " + filename);
        }
        return;
    }
    
    // クラウドストレージの場合
    provider->listFiles("root", [this, filename, callback, provider](bool success, juce::Array<R2CloudStorageProvider::FileInfo> files, juce::String errorMessage)
    {
        if (!success)
        {
            if (callback)
                callback(false, "", "Failed to list files: " + errorMessage);
            return;
        }
        
        juce::String fileId;
        for (const auto& file : files)
        {
            if (file.name == filename && !file.isFolder)
            {
                fileId = file.id;
                break;
            }
        }
        
        if (fileId.isEmpty())
        {
            if (callback)
                callback(false, "", "File not found: " + filename);
            return;
        }
        
        provider->downloadFile(fileId, [callback, filename](bool downloadSuccess, juce::MemoryBlock data, juce::String downloadError)
        {
            if (downloadSuccess)
            {
                auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                if (callback)
                    callback(true, content, "");
            }
            else
            {
                if (callback)
                    callback(false, "", "Failed to download file: " + downloadError);
            }
        });
    });
}

} // namespace r2juce
