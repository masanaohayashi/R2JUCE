#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>

namespace r2juce {

class R2CloudStorageProvider;
class R2CloudAuthComponent;

class R2CloudManager
{
public:
    R2CloudManager();
    ~R2CloudManager();
    
    enum class ServiceType
    {
        Local,
        GoogleDrive,
        OneDrive
    };
    
    enum class AuthStatus
    {
        NotAuthenticated,
        Authenticating,
        Authenticated,
        Error
    };
    
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setLocalStorageDirectory(const juce::File& directory);
    
    void selectService(ServiceType serviceType);
    ServiceType getCurrentService() const { return currentServiceType; }
    AuthStatus getAuthStatus() const;
    
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    using FileContentCallback = std::function<void(bool success, juce::String content, juce::String errorMessage)>;
    
    //==================== REFACTORED METHODS ====================
    /**
     * @brief 指定されたパスにファイルを保存します。
     * パスに'/'が含まれている場合、プロバイダはパスを解釈して適切なフォルダに保存します。
     * 含まれていない場合は、ルート直下に保存します。
     * @param filePath 保存するファイルのパス (e.g., "presets/bass/my_preset.txt" or "my_preset.txt")
     * @param content  保存するファイルの内容
     * @param callback 操作完了時のコールバック
     */
    void saveFile(const juce::String& filePath, const juce::String& content, FileOperationCallback callback = nullptr);

    /**
     * @brief 指定されたパスのファイルを読み込みます。
     * @param filePath 読み込むファイルのパス (e.g., "presets/bass/my_preset.txt" or "my_preset.txt")
     * @param callback 操作完了時のコールバック (成功時、ファイルの内容を含む)
     */
    void loadFile(const juce::String& filePath, FileContentCallback callback);
    //============================================================

    bool needsAuthentication() const;
    void showAuthenticationUI(juce::Component* parentComponent);
    void hideAuthenticationUI();
    void signOut();
    void cancelAuthentication();

    
    std::function<void(AuthStatus)> onAuthStatusChanged;
    std::function<void(ServiceType)> onServiceChanged;

private:
    ServiceType currentServiceType = ServiceType::Local;
    AuthStatus currentAuthStatus = AuthStatus::NotAuthenticated;
    
    // Providers
    std::unique_ptr<R2CloudStorageProvider> localProvider;
    std::unique_ptr<R2CloudStorageProvider> googleDriveProvider;
    std::unique_ptr<R2CloudStorageProvider> oneDriveProvider;
    
    std::unique_ptr<R2CloudAuthComponent> authComponent;
    juce::Component* parentForAuth = nullptr;
    
    // Settings
    juce::String googleClientId, googleClientSecret;
    juce::String oneDriveClientId, oneDriveClientSecret;
    juce::File localStorageDir;
    
    // Internal methods
    R2CloudStorageProvider* getCurrentProvider();
    const R2CloudStorageProvider* getCurrentProvider() const;
    void initializeProviders();
    void handleAuthenticationComplete(bool success, const juce::String& errorMessage,
                                     const juce::String& accessToken, const juce::String& refreshToken);
    void updateAuthStatus();
    void setAuthStatus(AuthStatus newStatus);
    void startDeviceFlowAuthentication(juce::Component* parent);
    
    // Factory pattern for future extensibility
    static std::unique_ptr<R2CloudStorageProvider> createProvider(ServiceType type);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CloudManager)
};

} // namespace r2juce
