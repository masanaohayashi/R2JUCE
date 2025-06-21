#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

class R2LocalStorageProvider : public R2CloudStorageProvider
{
public:
    R2LocalStorageProvider();
    ~R2LocalStorageProvider() override = default;
    
    // R2CloudStorageProvider interface
    void authenticate(AuthCallback callback) override;
    void signOut() override;
    Status getAuthStatus() const override;
    juce::String getDisplayName() const override;
    
    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                   const juce::String& folderId, FileOperationCallback callback) override;
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    void createFolder(const juce::String& folderName, const juce::String& parentId,
                     FileOperationCallback callback) override;
    
    // Local-specific methods
    void setRootDirectory(const juce::File& directory);
    juce::File getRootDirectory() const;
    
private:
    juce::File rootDirectory;
    juce::File getFileFromId(const juce::String& fileId) const;
    juce::String getIdFromFile(const juce::File& file) const;
};

} // namespace r2juce
