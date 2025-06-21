#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>

namespace r2juce {

class R2CloudStorageProvider;
class R2CloudAuthComponent;

/**
 * クラウドストレージ管理のメインAPIクラス
 * シンプルで使いやすいインターフェースを提供
 */
class R2CloudManager
{
public:
    R2CloudManager();
    ~R2CloudManager();
    
    enum class ServiceType
    {
        Local,
        GoogleDrive
        // 今後追加予定: iCloud, Dropbox, OneDrive
    };
    
    enum class AuthStatus
    {
        NotAuthenticated,
        Authenticating,
        Authenticated,
        Error
    };
    
    // === 設定API ===
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setLocalStorageDirectory(const juce::File& directory);
    // 今後追加予定: setDropboxCredentials, setOneDriveCredentials, setiCloudCredentials
    
    // === サービス選択・認証 ===
    void selectService(ServiceType serviceType);
    ServiceType getCurrentService() const { return currentServiceType; }
    AuthStatus getAuthStatus() const;
    
    // === ファイル操作（シンプルAPI） ===
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    using FileContentCallback = std::function<void(bool success, juce::String content, juce::String errorMessage)>;
    
    void saveFile(const juce::String& filename, const juce::String& content, FileOperationCallback callback = nullptr);
    void loadFile(const juce::String& filename, FileContentCallback callback);
    
    // === 認証関連 ===
    bool needsAuthentication() const;
    void showAuthenticationUI(juce::Component* parentComponent);
    void hideAuthenticationUI();
    void signOut();
    
    // === コールバック設定 ===
    std::function<void(AuthStatus)> onAuthStatusChanged;
    std::function<void(ServiceType)> onServiceChanged;

private:
    ServiceType currentServiceType = ServiceType::Local;
    AuthStatus currentAuthStatus = AuthStatus::NotAuthenticated;
    
    // Providers
    std::unique_ptr<R2CloudStorageProvider> localProvider;
    std::unique_ptr<R2CloudStorageProvider> googleDriveProvider;
    // 今後追加予定: iCloudProvider, dropboxProvider, oneDriveProvider
    
    // 認証UI
    std::unique_ptr<R2CloudAuthComponent> authComponent;
    juce::Component* parentForAuth = nullptr;
    
    // Settings
    juce::String googleClientId, googleClientSecret;
    juce::File localStorageDir;
    // 今後追加予定: dropbox, oneDrive, iCloud用の設定
    
    // Internal methods
    R2CloudStorageProvider* getCurrentProvider();
    const R2CloudStorageProvider* getCurrentProvider() const;
    void initializeProviders();
    void handleAuthenticationComplete(bool success, const juce::String& errorMessage,
                                     const juce::String& accessToken, const juce::String& refreshToken);
    void updateAuthStatus();
    void setAuthStatus(AuthStatus newStatus);
    
    // Factory pattern for future extensibility
    static std::unique_ptr<R2CloudStorageProvider> createProvider(ServiceType type);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CloudManager)
};

} // namespace r2juce
