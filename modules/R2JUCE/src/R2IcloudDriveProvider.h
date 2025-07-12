#pragma once

#include "R2CloudStorageProvider.h"
#include <memory>

// #if JUCE_MAC || JUCE_IOS

namespace r2juce {

class IcloudImpl;

class R2IcloudDriveProvider : public R2CloudStorageProvider {
public:
    /**
     * @brief Constructs an iCloud Drive provider with the necessary container ID.
     * @param containerId The identifier of the iCloud container to use.
     */
    R2IcloudDriveProvider(const juce::String& containerId);
    ~R2IcloudDriveProvider() override;

    //==============================================================================
    // R2CloudStorageProvider interface
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

private:
    std::unique_ptr<IcloudImpl> pimpl;
};

}  // namespace r2juce

// #endif  // JUCE_MAC || JUCE_IOS
