#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

/**
    @brief A concrete implementation of R2CloudStorageProvider for Google Drive.
    @details This class handles all interactions with the Google Drive API,
             including OAuth 2.0 authentication (token refresh), and file
             operations like upload, download, and listing.
*/
class R2GoogleDriveProvider : public R2CloudStorageProvider
{
public:
    R2GoogleDriveProvider();
    ~R2GoogleDriveProvider() override = default;

    //==============================================================================
    // R2CloudStorageProvider overrides
    void authenticate(AuthCallback callback) override;
    void signOut() override;
    Status getAuthStatus() const override;
    juce::String getDisplayName() const override;

    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) override;
    void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) override;

    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback) override;
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    void createFolder(const juce::String& folderName, const juce::String& parentId, FileOperationCallback callback) override;

    //==============================================================================
    /** @brief Sets the OAuth 2.0 client credentials required for API access.
        @param clientId     The client ID from Google API Console.
        @param clientSecret The client secret from Google API Console.
    */
    void setClientCredentials(const juce::String& clientId, const juce::String& clientSecret);

    /** @brief Manually sets the access and refresh tokens.
        @details This is typically used after the device flow authentication is complete.
        @param newAccessToken  The access token for API requests.
        @param newRefreshToken The refresh token to obtain new access tokens.
    */
    void setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken);

private:
    // Core method for making authenticated requests to the Google Drive API.
    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& httpMethod,
                        const juce::StringPairArray& headers,
                        const juce::MemoryBlock& postData,
                        std::function<void(bool success, juce::String response)> callback);

    // Overload for string-based post data.
    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& httpMethod,
                        const juce::StringPairArray& headers,
                        const juce::String& postData,
                        std::function<void(bool success, juce::String response)> callback);

    // Helper methods for file operations by path.
    void findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId, int pathIndex, DownloadCallback callback);
    void createFolderPath(const juce::StringArray& folderPath, const juce::String& parentFolderId, int pathIndex, std::function<void(bool success, juce::String newFolderId, juce::String errorMessage)> callback);
    
    // Helper methods for upload logic.
    void uploadToFolder(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback);
    void uploadNewFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback);
    void updateExistingFile(const juce::String& fileId, const juce::MemoryBlock& data, FileOperationCallback callback);

    // Token management methods.
    bool isTokenValid() const;
    void refreshAccessToken(std::function<void(bool success)> callback);
    void parseTokenResponse(const juce::String& response, std::function<void(bool success, juce::String errorMessage)> onComplete);

    // Token persistence methods.
    void saveTokens();
    bool loadTokens();
    juce::File getTokenFile() const;
    
    juce::String clientId;
    juce::String clientSecret;
    juce::String accessToken;
    juce::String refreshToken;
    juce::Time tokenExpiry;
};

} // namespace r2juce
