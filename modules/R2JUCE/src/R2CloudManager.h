#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>
#include "R2CloudStorageProvider.h"

namespace r2juce {

class R2CloudAuthComponent;

/**
    @brief Manages different cloud storage providers and handles the authentication process.
    @details This class provides a unified interface for interacting with various cloud
             services like Google Drive and OneDrive, as well as local storage. It
             manages service selection, authentication state, and delegates file
             operations to the currently active provider.
*/
class R2CloudManager
{
public:
    R2CloudManager();
    ~R2CloudManager();
    
    using ServiceType = R2CloudStorageProvider::ServiceType;
    
    enum class AuthStatus { NotAuthenticated, Authenticating, Authenticated, Error };

    struct AppState
    {
        ServiceType selectedService = ServiceType::Local;
        AuthStatus authStatus = AuthStatus::NotAuthenticated;
        juce::String statusLabelText;
        bool isSignOutButtonEnabled = false;
        bool areFileButtonsEnabled = false;
        bool isComboBoxEnabled = true;
        bool needsAuthUi = false;
    };
    
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setAppGroupId(const juce::String& groupId);

#if JUCE_MAC || JUCE_IOS
    void setIcloudContainerId(const juce::String& containerId);
#endif
    
    void userSelectedService(ServiceType serviceType);
    void userSignedOut();
    void userCancelledAuthentication();
    void authenticationFinished(bool success, const juce::String& errorMessage, const juce::String& accessToken, const juce::String& refreshToken);
    
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    using FileContentCallback = std::function<void(bool success, juce::String content, juce::String errorMessage)>;
    
    void saveFile(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback = nullptr);
    void loadFile(const juce::String& filePath, FileContentCallback callback);
    
    const AppState& getInitialState() const;
    const AppState& getCurrentState() const;

    std::function<void(const AppState&)> onStateChanged;
    
    void initializeProviders();

private:
    void startAuthenticationFlow();
    void refreshStateAndNotify();
    
    std::shared_ptr<R2CloudStorageProvider> createProvider(ServiceType type);

    AppState currentState;
    ServiceType previousServiceBeforeAuth = ServiceType::Local;

    std::shared_ptr<R2CloudStorageProvider> localProvider;
    std::shared_ptr<R2CloudStorageProvider> googleDriveProvider;
    std::shared_ptr<R2CloudStorageProvider> oneDriveProvider;
    
    juce::String googleClientId, googleClientSecret;
    juce::String oneDriveClientId, oneDriveClientSecret;
    juce::String appGroupId;
    
#if JUCE_MAC || JUCE_IOS
    juce::String iCloudContainerId;
    std::shared_ptr<R2CloudStorageProvider> iCloudDriveProvider;
#endif
    
    std::shared_ptr<R2CloudStorageProvider> getCurrentProvider();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CloudManager)
};

} // namespace r2juce

