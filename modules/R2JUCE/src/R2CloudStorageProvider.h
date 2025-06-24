#pragma once

#include <JuceHeader.h>
#include <functional>
#include <memory>

namespace r2juce {

/**
    @brief An abstract base class defining the interface for a cloud storage service provider.
    @details This class provides a common set of functionalities for authentication,
             file/folder manipulation, and state management that must be implemented
             by concrete provider classes like R2GoogleDriveProvider. It uses
             std::enable_shared_from_this to safely manage object lifetime in
             asynchronous operations.
*/
class R2CloudStorageProvider : public std::enable_shared_from_this<R2CloudStorageProvider>
{
public:
    virtual ~R2CloudStorageProvider() = default;
    
    /** @brief Represents the authentication status of the provider. */
    enum class Status { NotAuthenticated, Authenticating, Authenticated, Error };
    
    /** @brief A structure to hold metadata for a file or folder. */
    struct FileInfo
    {
        juce::String name;
        juce::String id;
        juce::String mimeType;
        juce::Time modifiedTime;
        int64_t size = 0;
        bool isFolder = false;
    };
    
    /** @brief Callback for authentication operations. */
    using AuthCallback = std::function<void(bool success, juce::String errorMessage)>;
    /** @brief Callback for listing files. */
    using FileListCallback = std::function<void(bool success, juce::Array<FileInfo> files, juce::String errorMessage)>;
    /** @brief Callback for file operations that don't return data (e.g., upload, delete). */
    using FileOperationCallback = std::function<void(bool success, juce::String errorMessage)>;
    /** @brief Callback for file download operations. */
    using DownloadCallback = std::function<void(bool success, juce::MemoryBlock data, juce::String errorMessage)>;
    
    /** @brief Initiates the authentication process for the service. */
    virtual void authenticate(AuthCallback callback) = 0;
    
    /** @brief Signs out from the service, clearing any stored credentials. */
    virtual void signOut() = 0;
    
    /** @brief Returns the current authentication status. */
    virtual Status getAuthStatus() const = 0;
    
    /** @brief Returns the display name of the cloud service (e.g., "Google Drive"). */
    virtual juce::String getDisplayName() const = 0;
    
    /** @brief Uploads data to a file specified by its full path. May create folders as needed. */
    virtual void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) = 0;
    
    /** @brief Downloads a file specified by its full path. */
    virtual void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) = 0;

    /** @brief Lists files and folders within a specified folder.
        @param folderId The ID of the folder to list. Use an empty string for the root folder.
    */
    virtual void listFiles(const juce::String& folderId, FileListCallback callback) = 0;

    /** @brief Uploads data as a new file to a specific folder.
        @param fileName The name of the file to create.
        @param data The content of the file.
        @param folderId The ID of the parent folder.
    */
    virtual void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                            const juce::String& folderId, FileOperationCallback callback) = 0;

    /** @brief Downloads a file by its unique ID. */
    virtual void downloadFile(const juce::String& fileId, DownloadCallback callback) = 0;

    /** @brief Deletes a file by its unique ID. */
    virtual void deleteFile(const juce::String& fileId, FileOperationCallback callback) = 0;

    /** @brief Creates a new folder.
        @param folderName The name of the new folder.
        @param parentId The ID of the parent folder.
    */
    virtual void createFolder(const juce::String& folderName, const juce::String& parentId,
                              FileOperationCallback callback) = 0;
                              
protected:
    Status currentStatus = Status::NotAuthenticated;
};
}
