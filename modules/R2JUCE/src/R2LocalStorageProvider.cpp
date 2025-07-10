#include "R2LocalStorageProvider.h"

namespace r2juce {

R2LocalStorageProvider::R2LocalStorageProvider()
{
    currentStatus = Status::Authenticated;
}

void R2LocalStorageProvider::authenticate(AuthCallback callback)
{
    if (callback)
        juce::MessageManager::callAsync([callback]() { callback(true, ""); });
}

void R2LocalStorageProvider::signOut()
{
    // No-op for local storage.
}

R2CloudStorageProvider::Status R2LocalStorageProvider::getAuthStatus() const
{
    return Status::Authenticated;
}

juce::String R2LocalStorageProvider::getDisplayName() const
{
    return "Local Storage";
}

R2CloudStorageProvider::ServiceType R2LocalStorageProvider::getServiceType() const
{
    return ServiceType::Local;
}

void R2LocalStorageProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    juce::Thread::launch([this, filePath, data, callback]() {
        auto targetFile = getLocalStorageRoot().getChildFile(filePath);
        targetFile.getParentDirectory().createDirectory();
        
        bool success = targetFile.replaceWithData(data.getData(), data.getSize());
        juce::String message = success ? "" : "Failed to write file to local storage.";
        
        if (callback)
            juce::MessageManager::callAsync([callback, success, message]() { callback(success, message); });
    });
}

void R2LocalStorageProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    juce::Thread::launch([this, filePath, callback]() {
        auto sourceFile = getLocalStorageRoot().getChildFile(filePath);
        if (!sourceFile.existsAsFile())
        {
            if (callback)
                juce::MessageManager::callAsync([callback, filePath]() { callback(false, {}, "File not found: " + filePath); });
            return;
        }
        
        juce::MemoryBlock data;
        bool success = sourceFile.loadFileAsData(data);
        juce::String message = success ? "" : "Failed to read file from local storage.";

        if (callback)
            juce::MessageManager::callAsync([callback, success, data, message]() { callback(success, data, message); });
    });
}

juce::File R2LocalStorageProvider::getLocalStorageRoot() const
{
    auto root = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("R2JuceCloudApp")
                .getChildFile("LocalStorage");
    
    if (!root.exists())
        root.createDirectory();
        
    return root;
}

void R2LocalStorageProvider::listFiles(const juce::String&, FileListCallback callback)
{
    if (callback) juce::MessageManager::callAsync([callback]() { callback(false, {}, "Not implemented for local storage"); });
}

void R2LocalStorageProvider::uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback)
{
    uploadFileByPath(juce::File(folderId).getChildFile(fileName).getFullPathName(), data, callback);
}

void R2LocalStorageProvider::downloadFile(const juce::String& fileId, DownloadCallback callback)
{
    downloadFileByPath(fileId, callback);
}

void R2LocalStorageProvider::deleteFile(const juce::String& fileId, FileOperationCallback callback)
{
    juce::Thread::launch([this, fileId, callback]() {
        auto fileToDelete = getLocalStorageRoot().getChildFile(fileId);
        bool success = fileToDelete.deleteFile();
        juce::String message = success ? "" : "Failed to delete file.";
        if (callback)
            juce::MessageManager::callAsync([callback, success, message]() { callback(success, message); });
    });
}

void R2LocalStorageProvider::createFolder(const juce::String& folderName, const juce::String& parentId, FileOperationCallback callback)
{
    juce::Thread::launch([this, folderName, parentId, callback]() {
        auto parentDir = getLocalStorageRoot().getChildFile(parentId);
        bool success = parentDir.getChildFile(folderName).createDirectory();
        juce::String message = success ? "" : "Failed to create directory.";
        if (callback)
            juce::MessageManager::callAsync([callback, success, message]() { callback(success, message); });
    });
}

} // namespace r2juce

