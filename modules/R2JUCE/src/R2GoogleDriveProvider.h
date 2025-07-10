#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

class R2GoogleDriveProvider : public R2CloudStorageProvider
{
public:
    R2GoogleDriveProvider();
    ~R2GoogleDriveProvider() override = default;

    //==============================================================================
    void authenticate(AuthCallback callback) override;
    void signOut() override;
    Status getAuthStatus() const override;
    juce::String getDisplayName() const override;
    ServiceType getServiceType() const override; // This is the required override

    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) override;
    void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) override;

    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback) override;
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    void createFolder(const juce::String& folderName, const juce::String& parentId, FileOperationCallback callback) override;

    //==============================================================================
    void setClientCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken);

private:
    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& httpMethod,
                        const juce::StringPairArray& headers,
                        const juce::MemoryBlock& postData,
                        std::function<void(bool success, juce::String response)> callback);

    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& httpMethod,
                        const juce::StringPairArray& headers,
                        const juce::String& postData,
                        std::function<void(bool success, juce::String response)> callback);

    void findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId, int pathIndex, DownloadCallback callback);
    void createFolderPath(const juce::StringArray& folderPath, const juce::String& parentFolderId, int pathIndex, std::function<void(bool success, juce::String newFolderId, juce::String errorMessage)> callback);
    
    void uploadToFolder(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback);
    void uploadNewFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback);
    void updateExistingFile(const juce::String& fileId, const juce::MemoryBlock& data, FileOperationCallback callback);

    bool isTokenValid() const;
    void refreshAccessToken(std::function<void(bool success)> callback);
    void parseTokenResponse(const juce::String& response, std::function<void(bool success, juce::String errorMessage)> onComplete);

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

