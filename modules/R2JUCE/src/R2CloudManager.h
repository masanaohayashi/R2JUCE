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
    
    enum class ServiceType { Local, GoogleDrive, OneDrive };
    enum class AuthStatus { NotAuthenticated, Authenticating, Authenticated, Error };
    
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);
    
    void selectService(ServiceType serviceType);
    ServiceType getCurrentService() const { return currentServiceType; }
    AuthStatus getAuthStatus() const;
    
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    using FileContentCallback = std::function<void(bool success, juce::String content, juce::String errorMessage)>;
    
    void saveFile(const juce::String& filePath, const juce::String& content, FileOperationCallback callback = nullptr);
    void loadFile(const juce::String& filePath, FileContentCallback callback);

    bool needsAuthentication() const;
    void showAuthenticationUI(juce::Component* parentComponent);
    void hideAuthenticationUI();
    void signOut();
    
    std::function<void(AuthStatus)> onAuthStatusChanged;
    std::function<void(ServiceType)> onServiceChanged;

private:
    ServiceType currentServiceType = ServiceType::Local;
    AuthStatus currentAuthStatus = AuthStatus::NotAuthenticated;
    
    std::shared_ptr<R2CloudStorageProvider> localProvider;
    std::shared_ptr<R2CloudStorageProvider> googleDriveProvider;
    std::shared_ptr<R2CloudStorageProvider> oneDriveProvider;
    
    std::unique_ptr<R2CloudAuthComponent> authComponent;
    juce::Component* parentForAuth = nullptr;
    
    juce::String googleClientId, googleClientSecret;
    juce::String oneDriveClientId, oneDriveClientSecret;
    
    std::shared_ptr<R2CloudStorageProvider> getCurrentProvider();
    std::shared_ptr<const R2CloudStorageProvider> getCurrentProvider() const;
    void initializeProviders();
    void handleAuthenticationComplete(bool success, const juce::String& errorMessage, const juce::String& accessToken, const juce::String& refreshToken);
    void updateAuthStatus();
    void setAuthStatus(AuthStatus newStatus);
    void startDeviceFlowAuthentication(juce::Component* parent);
    
    static std::shared_ptr<R2CloudStorageProvider> createProvider(ServiceType type);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CloudManager)
};
}
