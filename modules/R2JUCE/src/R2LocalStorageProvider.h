#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

class R2LocalStorageProvider : public R2CloudStorageProvider
{
public:
    R2LocalStorageProvider();
    
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
                      
    void setRootDirectory(const juce::File& directory);
    juce::File getRootDirectory() const;
    juce::File getFileFromId(const juce::String& fileId) const;
    juce::String getIdFromFile(const juce::File& file) const;

private:
    juce::File rootDirectory;
};

} // namespace r2juce
