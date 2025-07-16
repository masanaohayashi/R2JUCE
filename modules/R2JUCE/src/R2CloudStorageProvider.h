#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <memory>

namespace r2juce {

class R2LocalStorageProvider;

class R2CloudStorageProvider : public std::enable_shared_from_this<R2CloudStorageProvider>
{
public:
    virtual ~R2CloudStorageProvider() = default;
    
    /** @brief Identifies the available storage services. */
    enum class ServiceType { Local, iCloudDrive, GoogleDrive, OneDrive };

    enum class Status { NotAuthenticated, Authenticating, Authenticated, Error };

    /** @brief Represents the status of background synchronization when caching is enabled. */
    enum class SyncStatus { Idle, Syncing, Synced, SyncError };
    
    struct FileInfo
    {
        juce::String name;
        juce::String id;
        juce::String mimeType;
        juce::Time modifiedTime;
        int64_t size = 0;
        bool isFolder = false;
    };
    
    using AuthCallback = std::function<void(bool success, juce::String errorMessage)>;
    using FileListCallback = std::function<void(bool success, juce::Array<FileInfo> files, juce::String errorMessage)>;
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    using DownloadCallback = std::function<void(bool success, juce::MemoryBlock data, juce::String errorMessage)>;
    
    virtual void authenticate(AuthCallback callback) = 0;
    virtual void signOut() = 0;
    virtual Status getAuthStatus() const = 0;
    virtual juce::String getDisplayName() const = 0;
    
    /** @brief Returns the type of the storage service. This is key to avoiding dynamic_cast. */
    virtual ServiceType getServiceType() const = 0;

    virtual void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) = 0;

    /**
     * @brief Uploads a file synchronously, blocking until the operation is complete.
     * @param filePath The destination path for the file in the cloud.
     * @param data The file content to upload.
     * @return True if the upload was successful, false otherwise.
     * @note This method performs a blocking network operation and should not be called on the message thread during normal UI interaction.
     */
    virtual bool uploadFileByPathSync(const juce::String& filePath, const juce::MemoryBlock& data)
    {
        // Default implementation for providers that don't support sync operations.
        // Must be overridden by concrete remote providers like GoogleDrive and OneDrive.
        juce::ignoreUnused(filePath, data);
        return false;
    }

    virtual void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) = 0;
    virtual void listFiles(const juce::String& folderId, FileListCallback callback) = 0;
    virtual void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                            const juce::String& folderId, FileOperationCallback callback) = 0;
    virtual void downloadFile(const juce::String& fileId, DownloadCallback callback) = 0;
    virtual void deleteFile(const juce::String& fileId, FileOperationCallback callback) = 0;
    virtual void createFolder(const juce::String& folderName, const juce::String& parentId,
                              FileOperationCallback callback) = 0;
                              
    /**
     * @brief Configures caching for this provider.
     * @param cacheProvider A pointer to the local storage provider to use for caching.
     * If this is nullptr, caching is disabled.
     */
    virtual void configureCaching(std::shared_ptr<R2LocalStorageProvider> cacheProvider)
    {
        juce::ignoreUnused(cacheProvider);
    }

    /**
     * @brief Checks if caching is currently enabled for this provider.
     * @return True if caching is enabled, false otherwise.
     */
    virtual bool isCachingEnabled() const
    {
        return false;
    }

    /** @brief Callback to report changes in the background sync status. */
    std::function<void(SyncStatus, const juce::String&)> onSyncStatusChanged;

protected:
    Status currentStatus = Status::NotAuthenticated;
};
}

