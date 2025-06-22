#include "R2CloudManager.h"
#include "R2LocalStorageProvider.h"
#include "R2GoogleDriveProvider.h"
#include "R2OneDriveProvider.h"  // ← 追加
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
    oneDriveProvider = createProvider(ServiceType::OneDrive);  // ← 追加
    
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
        case ServiceType::OneDrive:  // ← 追加
            return std::make_unique<R2OneDriveProvider>();
        case ServiceType::Local:
        default:
            return std::make_unique<R2LocalStorageProvider>();
            // 今後追加予定:
            // case ServiceType::Dropbox: return std::make_unique<R2DropboxProvider>();
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

void R2CloudManager::setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret)  // ← 追加
{
    oneDriveClientId = clientId;
    oneDriveClientSecret = clientSecret;
    
    if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
    {
        oneDriveProv->setClientCredentials(clientId, clientSecret);
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
        case ServiceType::OneDrive:  // ← 追加
            return oneDriveProvider.get();
        case ServiceType::Local:
        default:
            return localProvider.get();
            // 今後追加予定:
            // case ServiceType::iCloud: return iCloudProvider.get();
            // case ServiceType::Dropbox: return dropboxProvider.get();
    }
}

const R2CloudStorageProvider* R2CloudManager::getCurrentProvider() const
{
    switch (currentServiceType)
    {
        case ServiceType::GoogleDrive:
            return googleDriveProvider.get();
        case ServiceType::OneDrive:  // ← 追加
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
    else if (currentServiceType == ServiceType::OneDrive)  // ← 追加
    {
        authComponent->setOneDriveCredentials(oneDriveClientId, oneDriveClientSecret);
        authComponent->setServiceType(R2CloudAuthComponent::ServiceType::OneDrive);
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
        if (currentServiceType == ServiceType::GoogleDrive)
        {
            if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(googleDriveProvider.get()))
            {
                googleProv->setTokens(accessToken, refreshToken);
            }
        }
        else if (currentServiceType == ServiceType::OneDrive)
        {
            if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(oneDriveProvider.get()))
            {
                oneDriveProv->setTokens(accessToken, refreshToken);
            }
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

void R2CloudManager::saveFileWithPath(const juce::String& folderPath, const juce::String& filename,
                                      const juce::String& content, FileOperationCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider)
    {
        if (callback) callback(false, "No storage provider available");
        return;
    }
    
    if (needsAuthentication())
    {
        if (callback) callback(false, "Authentication required");
        return;
    }
    
    juce::MemoryBlock data(content.toUTF8(), content.getNumBytesAsUTF8());
    
    // OneDriveの場合はパス形式で直接アップロード可能
    juce::String fullPath = folderPath + "/" + filename;
    provider->uploadFile(fullPath, data, "path", [callback](bool success, juce::String errorMessage)
                         {
        if (callback) callback(success, errorMessage);
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
    
    // OneDrive/Google Drive: パス形式に対応
    if (filename.contains("/"))
    {
        // パス形式での直接読み込み
        loadFileWithPath(filename, callback);
    }
    else
    {
        // 従来のルートからの検索
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
}

void R2CloudManager::loadFileWithPath(const juce::String& filePath, FileContentCallback callback)
{
    auto provider = getCurrentProvider();
    if (!provider) return;
    
    // OneDrive用の直接パス読み込み
    if (auto* oneDriveProv = dynamic_cast<R2OneDriveProvider*>(provider))
    {
        oneDriveProv->downloadFileWithPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String error)
        {
            if (success)
            {
                auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                if (callback) callback(true, content, "");
            }
            else
            {
                if (callback) callback(false, "", error);
            }
        });
    }
    // Google Drive用のパス読み込み
    else if (auto* googleProv = dynamic_cast<R2GoogleDriveProvider*>(provider))
    {
        googleProv->downloadFileWithPath(filePath, [callback](bool success, juce::MemoryBlock data, juce::String error)
        {
            if (success)
            {
                auto content = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                if (callback) callback(true, content, "");
            }
            else
            {
                if (callback) callback(false, "", error);
            }
        });
    }
}

} // namespace r2juce
