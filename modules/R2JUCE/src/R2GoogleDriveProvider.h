#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {
class R2GoogleDriveProvider : public R2CloudStorageProvider
{
public:
    R2GoogleDriveProvider();
    ~R2GoogleDriveProvider() override = default;
    
    // R2CloudStorageProvider interface
    void authenticate(AuthCallback callback) override;
    void signOut() override;
    Status getAuthStatus() const override;
    juce::String getDisplayName() const override;
    
    // ==================== IMPLEMENTING NEW INTERFACE ====================
    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) override;
    void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) override;
    // ==================================================================
    
    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                    const juce::String& folderId, FileOperationCallback callback) override;
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    void createFolder(const juce::String& folderName, const juce::String& parentId,
                      FileOperationCallback callback) override;
    
    // Google Drive specific
    void setClientCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void exchangeAuthCodeForTokens(const juce::String& authCode, std::function<void(bool, juce::String)> callback);
    void setTokens(const juce::String& accessToken, const juce::String& refreshToken);
    void cancelAuthentication();

private:
    class OAuth2Handler; // Forward declaration

    using FolderCreationCallback = std::function<void(bool success, juce::String folderId, juce::String errorMessage)>;

    void startNewAuthFlow(AuthCallback callback);
    void parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> callback);
    void findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId,
                           int pathIndex, DownloadCallback callback);
    void uploadToFolder(const juce::String& fileName, const juce::MemoryBlock& data,
                       const juce::String& folderId, FileOperationCallback callback);
    void uploadWithData(const juce::String& endpoint, const juce::String& boundary,
                        const juce::MemoryBlock& fullData, FileOperationCallback callback);
    void updateExistingFile(const juce::String& fileId, const juce::MemoryBlock& data,
                           FileOperationCallback callback);
    void uploadNewFile(const juce::String& fileName, const juce::MemoryBlock& data,
                      const juce::String& folderId, FileOperationCallback callback);
    void createFolderPath(const juce::StringArray& folderPath, const juce::String& parentFolderId,
                         int pathIndex, std::function<void(bool, juce::String, juce::String)> callback);

    std::unique_ptr<OAuth2Handler> oauth2Handler;
    
    juce::String clientId;
    juce::String clientSecret;
    juce::String accessToken;
    juce::String refreshToken;
    juce::Time tokenExpiry;
    
    bool isTokenValid() const;
    void refreshAccessToken(std::function<void(bool)> callback);
    void makeAPIRequest(const juce::String& endpoint, const juce::String& method,
                        const juce::StringPairArray& headers, const juce::String& postData,
                        std::function<void(bool, juce::String)> callback);
    
    void saveTokens();
    bool loadTokens();
    juce::File getTokenFile() const;
};
}   //  namespace r2juce
