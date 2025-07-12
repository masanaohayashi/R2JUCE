#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

class R2OneDriveProvider : public R2CloudStorageProvider
{
public:
    /**
     * @brief Constructs an OneDrive provider with the necessary client credentials.
     * @param clientId The Application (client) ID from the Azure portal.
     * @param clientSecret The client secret value.
     */
    R2OneDriveProvider(const juce::String& clientId, const juce::String& clientSecret);
    ~R2OneDriveProvider() override = default;
    
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
    void setTokens(const juce::String& accessToken, const juce::String& refreshToken);

private:
    juce::String clientId, clientSecret, accessToken, refreshToken;
    juce::Time tokenExpiry;
    
    void createFolderPath(const juce::StringArray& folderPath, const juce::String& parentId, int pathIndex, std::function<void(bool, juce::String)> callback);
    void uploadToFolder(const juce::String& fileName, const juce::String& parentId, const juce::MemoryBlock& data, FileOperationCallback callback);

    bool isTokenValid() const;
    void refreshAccessToken(AuthCallback callback);
    
    void makeAPIRequest(const juce::String& endpoint,
                        const juce::String& method,
                        const juce::StringPairArray& headers,
                        const juce::MemoryBlock& body,
                        std::function<void(bool, int, const juce::var&)> callback);

    void saveTokens();
    bool loadTokens();
    juce::File getTokenFile() const;
};
}

