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
    
    /** @brief Sets the OAuth 2.0 credentials for Google Drive.
        @param clientId     The Google API client ID.
        @param clientSecret The Google API client secret.
    */
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);

    /** @brief Sets the OAuth 2.0 credentials for Microsoft OneDrive.
        @param clientId     The OneDrive application (client) ID.
        @param clientSecret The OneDrive client secret.
    */
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);
    
    /** @brief Selects the active cloud storage service.
        @param serviceType The service to activate.
    */
    void selectService(ServiceType serviceType);
    
    /** @brief Gets the currently selected service type.
        @return The current ServiceType.
    */
    ServiceType getCurrentService() const { return currentServiceType; }
    
    /** @brief Gets the current authentication status.
        @return The current AuthStatus.
    */
    AuthStatus getAuthStatus() const;
    
    /** @brief Defines a callback function for asynchronous file operations that do not return content.
        @param success      True if the operation was successful.
        @param errorMessage An error message if the operation failed.
    */
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    
    /** @brief Defines a callback function for asynchronous file loading operations.
        @param success      True if the operation was successful.
        @param content      The content of the loaded file.
        @param errorMessage An error message if the operation failed.
    */
    using FileContentCallback = std::function<void(bool success, juce::String content, juce::String errorMessage)>;
    
    /** @brief Saves content to a file in the currently selected cloud service.
        @param filePath     The path (or identifier) of the file in the cloud storage.
        @param data         The binary content to save as a MemoryBlock.
        @param callback     An optional callback to be invoked upon completion.
    */
    // Modified: contentをjuce::MemoryBlockで受け取る
    void saveFile(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback = nullptr);

    /** @brief Loads the content of a file from the currently selected cloud service.
        @param filePath     The path (or identifier) of the file in the cloud storage.
        @param callback     A callback to be invoked with the file content upon completion.
    */
    void loadFile(const juce::String& filePath, FileContentCallback callback);

    /** @brief Checks if the current service requires authentication.
        @return True if the user is not authenticated for the current service, false otherwise.
    */
    bool needsAuthentication() const;
    
    /** @brief Displays the authentication UI for the current service if needed.
        @param parentComponent The component that will host the authentication UI.
    */
    void showAuthenticationUI(juce::Component* parentComponent);

    /** @brief Hides and destroys the authentication UI component. */
    void hideAuthenticationUI();
    
    /** @brief Signs out from the current service and clears authentication tokens. */
    void signOut();
    
    /** @brief A callback that is triggered when the authentication status changes.
        @param newStatus The new AuthStatus.
    */
    std::function<void(AuthStatus)> onAuthStatusChanged;

    /** @brief A callback that is triggered when the selected service changes.
        @param serviceType The newly selected ServiceType.
    */
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
