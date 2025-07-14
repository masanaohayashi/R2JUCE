#pragma once

#include "R2CloudStorageProvider.h"
#include <atomic>

namespace r2juce {

class R2LocalStorageProvider;

class R2GoogleDriveProvider : public R2CloudStorageProvider
{
public:
    R2GoogleDriveProvider(const juce::String& clientId, const juce::String& clientSecret);
    ~R2GoogleDriveProvider() override;

    //==============================================================================
    void authenticate(AuthCallback callback) override;
    void signOut() override;
    Status getAuthStatus() const override;
    juce::String getDisplayName() const override;
    ServiceType getServiceType() const override;

    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) override;
    void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) override;

    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback) override;
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    void createFolder(const juce::String& folderName, const juce::String& parentId, FileOperationCallback callback) override;

    //==============================================================================
    void setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken);

    //== Caching ===================================================================
    void configureCaching(std::shared_ptr<R2LocalStorageProvider> cacheProvider) override;
    bool isCachingEnabled() const override;

private:
    void uploadDirectToCloud(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback);

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

    void findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId, int pathIndex, DownloadCallback callback, bool metadataOnly = false);
    void getCloudFileMetadata(const juce::String& filePath, std::function<void(bool, const FileInfo&)> callback);

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

    //== Caching Implementation ==================================================
    void triggerBackgroundUpload(const juce::String& filePath);
    void performUpload();
    
    juce::String clientId;
    juce::String clientSecret;
    juce::String accessToken;
    juce::String refreshToken;
    juce::Time tokenExpiry;

    std::shared_ptr<R2LocalStorageProvider> cacheProvider;

    juce::CriticalSection uploadLock;
    bool isUploading = false;
    juce::String pendingUploadPath;
};

} // namespace r2juce

