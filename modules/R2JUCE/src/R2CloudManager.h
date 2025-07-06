#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <functional>

namespace r2juce {

class R2CloudStorageProvider;
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
    
    /** @brief Identifies the available storage services. */
    enum class ServiceType { Local, GoogleDrive, OneDrive };
    
    /** @brief Represents the current authentication status for the selected service. */
    enum class AuthStatus { NotAuthenticated, Authenticating, Authenticated, Error };

    /** @brief A structure representing the entire state of the application UI. */
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
    
    /** @brief Sets the OAuth 2.0 credentials for Google Drive. */
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);

    /** @brief Sets the OAuth 2.0 credentials for Microsoft OneDrive. */
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);
    
    /** @brief Called by the UI when the user selects a service. */
    void userSelectedService(ServiceType serviceType);
    
    /** @brief Called by the UI when the user clicks the Sign Out button. */
    void userSignedOut();

    /** @brief Called by the UI when the user cancels the authentication flow. */
    void userCancelledAuthentication();

    /** @brief Called by the UI when the authentication flow completes. */
    void authenticationFinished(bool success, const juce::String& errorMessage, const juce::String& accessToken, const juce::String& refreshToken);
    
    /** @brief Defines a callback function for asynchronous file operations that do not return content. */
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    
    /** @brief Defines a callback function for asynchronous file loading operations. */
    using FileContentCallback = std::function<void(bool success, juce::String content, juce::String errorMessage)>;
    
    /** @brief Saves content to a file in the currently selected cloud service. */
    void saveFile(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback = nullptr);

    /** @brief Loads the content of a file from the currently selected cloud service. */
    void loadFile(const juce::String& filePath, FileContentCallback callback);
    
    /** @brief Returns the initial state of the application. */
    const AppState& getInitialState() const;

    /** @brief Returns the current state of the application. */
    const AppState& getCurrentState() const;

    /** @brief A callback that is triggered whenever the application state changes. */
    std::function<void(const AppState&)> onStateChanged;

private:
    void initializeProviders();
    void startAuthenticationFlow();
    void refreshStateAndNotify();
    
    static std::shared_ptr<R2CloudStorageProvider> createProvider(ServiceType type);

    AppState currentState;
    ServiceType previousServiceBeforeAuth = ServiceType::Local;

    std::shared_ptr<R2CloudStorageProvider> localProvider;
    std::shared_ptr<R2CloudStorageProvider> googleDriveProvider;
    std::shared_ptr<R2CloudStorageProvider> oneDriveProvider;
    
    juce::String googleClientId, googleClientSecret;
    juce::String oneDriveClientId, oneDriveClientSecret;
    
    std::shared_ptr<R2CloudStorageProvider> getCurrentProvider();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2CloudManager)
};
}

