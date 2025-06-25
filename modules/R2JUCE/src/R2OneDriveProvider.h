#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

/**
    @brief A concrete implementation of R2CloudStorageProvider for Microsoft OneDrive.
    @details This class handles interactions with the Microsoft Graph API to provide
             file storage services. It manages OAuth 2.0 authentication, including
             token refresh and persistence, and focuses primarily on path-based
             file operations.
*/
class R2OneDriveProvider : public R2CloudStorageProvider
{
public:
    R2OneDriveProvider();
    ~R2OneDriveProvider() override = default;
    
    //==============================================================================
    // R2CloudStorageProvider interface
    
    /** @brief Initiates the OneDrive authentication flow. (Inherited) */
    void authenticate(AuthCallback callback) override;
    
    /** @brief Signs out from OneDrive by clearing all local tokens. (Inherited) */
    void signOut() override;
    
    /** @brief Returns the current authentication status. (Inherited) */
    Status getAuthStatus() const override;
    
    /** @brief Returns the display name "OneDrive". (Inherited) */
    juce::String getDisplayName() const override;
    
    /** @brief Uploads data to a file specified by its full path in OneDrive, creating parent folders as needed. (Inherited) */
    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) override;
    
    /** @brief Downloads a file specified by its full path from OneDrive. (Inherited) */
    void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) override;

    /** @brief Not yet implemented. Returns an error immediately. (Inherited) */
    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    
    /** @brief Not yet implemented. Returns an error immediately. (Inherited) */
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback) override;
    
    /** @brief Not yet implemented. Returns an error immediately. (Inherited) */
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    
    /** @brief Not yet implemented. Returns an error immediately. (Inherited) */
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    
    /** @brief Not yet implemented. Returns an error immediately. (Inherited) */
    void createFolder(const juce::String& folderName, const juce::String& parentId, FileOperationCallback callback) override;
    
    //==============================================================================
    // OneDrive specific methods
    
    /** @brief Sets the OAuth 2.0 client credentials for the Microsoft Graph API.
        @param clientId     The Application (client) ID from the Azure portal.
        @param clientSecret The client secret value.
    */
    void setClientCredentials(const juce::String& clientId, const juce::String& clientSecret);
    
    /** @brief Manually sets the access and refresh tokens after a successful authentication flow.
        @param accessToken  The access token for API requests.
        @param refreshToken The token used to obtain new access tokens.
    */
    void setTokens(const juce::String& accessToken, const juce::String& refreshToken);

private:
    juce::String clientId, clientSecret, accessToken, refreshToken;
    juce::Time tokenExpiry;
    
    // NOTE: tokenLock has been removed as per user instruction.
    // If multi-threading issues related to token access persist,
    // explicit synchronization should be reconsidered.

    // Recursively creates the folder structure for a given path.
    void createFolderPath(const juce::StringArray& folderPath, const juce::String& parentId, int pathIndex, std::function<void(bool, juce::String)> callback);
    // Helper to upload data to a specific folder after the path has been resolved.
    void uploadToFolder(const juce::String& fileName, const juce::String& parentId, const juce::MemoryBlock& data, FileOperationCallback callback);

    // Token management helpers
    bool isTokenValid() const;
    // FIX: Change refreshAccessToken signature to return errorMessage
    void refreshAccessToken(AuthCallback callback);
    
    // Core API request wrapper
    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& method,
                        const juce::StringPairArray& headers,
                        const juce::String& body,
                        std::function<void(bool, int, const juce::var&)> callback);
    
    /** @brief Core API request wrapper for binary data.
        @param endpoint The API endpoint URL.
        @param method The HTTP method (e.g., "PUT", "POST").
        @param headers HTTP headers to include in the request.
        @param body The request body as a MemoryBlock.
        @param callback The callback to be invoked with the result.
    */
    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& method,
                        const juce::StringPairArray& headers,
                        const juce::MemoryBlock& body,
                        std::function<void(bool, int, const juce::var&)> callback);

    // Token persistence helpers
    void saveTokens();
    bool loadTokens();
    juce::File getTokenFile() const;
};
}
