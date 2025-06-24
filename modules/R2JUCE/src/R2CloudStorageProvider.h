#pragma once

#include <JuceHeader.h>
#include <functional>
#include <memory>

namespace r2juce {

// FIX: Inherit from std::enable_shared_from_this to safely manage lifetime in async callbacks
class R2CloudStorageProvider : public std::enable_shared_from_this<R2CloudStorageProvider>
{
public:
    virtual ~R2CloudStorageProvider() = default;
    
    enum class Status { NotAuthenticated, Authenticating, Authenticated, Error };
    
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
    
    virtual void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) = 0;
    virtual void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) = 0;

    virtual void listFiles(const juce::String& folderId, FileListCallback callback) = 0;
    virtual void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                            const juce::String& folderId, FileOperationCallback callback) = 0;
    virtual void downloadFile(const juce::String& fileId, DownloadCallback callback) = 0;
    virtual void deleteFile(const juce::String& fileId, FileOperationCallback callback) = 0;
    virtual void createFolder(const juce::String& folderName, const juce::String& parentId,
                              FileOperationCallback callback) = 0;
                              
protected:
    Status currentStatus = Status::NotAuthenticated;
};
}
