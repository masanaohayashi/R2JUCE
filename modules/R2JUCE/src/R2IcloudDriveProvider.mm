// #if JUCE_MAC || JUCE_IOS

#import <Foundation/Foundation.h>
#include "R2IcloudDriveProvider.h"

namespace r2juce {

class IcloudImpl {
public:
    IcloudImpl(const juce::String& containerId) {
        rootContainerDirectory = getICloudContainerURLForIdentifier(containerId);
        if (rootContainerDirectory.exists()) {
            status = R2CloudStorageProvider::Status::Authenticated;
            displayName = "iCloud Drive";
        } else {
            status = R2CloudStorageProvider::Status::Error;
            displayName = "iCloud Drive (Not Available)";
        }
    }

    void authenticate(R2CloudStorageProvider::AuthCallback callback) {
        if (callback)
            callback(status == R2CloudStorageProvider::Status::Authenticated,
                     status == R2CloudStorageProvider::Status::Authenticated
                         ? ""
                         : "iCloud container not available.");
    }

    void signOut() { /* No-op */ }
    R2CloudStorageProvider::Status getStatus() const { return status; }
    juce::String getDisplayName() const { return displayName; }
    R2CloudStorageProvider::ServiceType getServiceType() const { return R2CloudStorageProvider::ServiceType::iCloudDrive; }

    // All file operations are now synchronous. Callbacks are posted to the message thread if provided.
    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, R2CloudStorageProvider::FileOperationCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, "iCloud not available"); });
            return;
        }
        auto targetFile = rootContainerDirectory.getChildFile(filePath);
        targetFile.getParentDirectory().createDirectory();
        bool success = targetFile.replaceWithData(data.getData(), data.getSize());
        if (callback) juce::MessageManager::callAsync([callback, success]() { callback(success, success ? "" : "Failed to write file to iCloud"); });
    }

    void downloadFileByPath(const juce::String& filePath, R2CloudStorageProvider::DownloadCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, {}, "iCloud not available"); });
            return;
        }
        auto file = rootContainerDirectory.getChildFile(filePath);
        if (!file.existsAsFile()) {
            if (callback) juce::MessageManager::callAsync([callback, filePath]() { callback(false, {}, "File not found in iCloud: " + filePath); });
            return;
        }
        juce::MemoryBlock data;
        bool success = file.loadFileAsData(data);
        if (callback) juce::MessageManager::callAsync([callback, success, data]() { callback(success, data, success ? "" : "Failed to read file from iCloud"); });
    }

    void listFiles(const juce::String& folderId, R2CloudStorageProvider::FileListCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, {}, "iCloud not available"); });
            return;
        }
        juce::Array<R2CloudStorageProvider::FileInfo> files;
        auto folder = getFileFromId(folderId);
        if (!folder.exists() || !folder.isDirectory()) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, {}, "Folder not found in iCloud"); });
            return;
        }
        for (const auto& child : folder.findChildFiles(juce::File::findFilesAndDirectories, false)) {
            R2CloudStorageProvider::FileInfo info;
            info.name = child.getFileName();
            info.id = getIdFromFile(child);
            info.isFolder = child.isDirectory();
            info.size = child.getSize();
            info.modifiedTime = child.getLastModificationTime();
            files.add(info);
        }
        if (callback) juce::MessageManager::callAsync([callback, files]() { callback(true, files, ""); });
    }

    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, R2CloudStorageProvider::FileOperationCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, "iCloud not available"); });
            return;
        }
        auto folder = getFileFromId(folderId);
        if (!folder.exists()) folder.createDirectory();
        auto targetFile = folder.getChildFile(fileName);
        bool success = targetFile.replaceWithData(data.getData(), data.getSize());
        if (callback) juce::MessageManager::callAsync([callback, success]() { callback(success, success ? "" : "Failed to write file to iCloud"); });
    }

    void downloadFile(const juce::String& fileId, R2CloudStorageProvider::DownloadCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, {}, "iCloud not available"); });
            return;
        }
        auto file = getFileFromId(fileId);
        if (!file.exists() || file.isDirectory()) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, {}, "File not found in iCloud"); });
            return;
        }
        juce::MemoryBlock data;
        bool success = file.loadFileAsData(data);
        if (callback) juce::MessageManager::callAsync([callback, success, data]() { callback(success, data, success ? "" : "Failed to read file from iCloud"); });
    }

    void deleteFile(const juce::String& fileId, R2CloudStorageProvider::FileOperationCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, "iCloud not available"); });
            return;
        }
        auto file = getFileFromId(fileId);
        bool success = file.exists() && file.deleteRecursively();
        if (callback) juce::MessageManager::callAsync([callback, success]() { callback(success, success ? "" : "Failed to delete file from iCloud"); });
    }

    void createFolder(const juce::String& folderName, const juce::String& parentId, R2CloudStorageProvider::FileOperationCallback callback)
    {
        if (status == R2CloudStorageProvider::Status::Error) {
            if (callback) juce::MessageManager::callAsync([callback]() { callback(false, "iCloud not available"); });
            return;
        }
        auto parent = getFileFromId(parentId);
        auto newFolder = parent.getChildFile(folderName);
        bool success = newFolder.createDirectory();
        if (callback) juce::MessageManager::callAsync([callback, success]() { callback(success, success ? "" : "Failed to create folder in iCloud"); });
    }

private:
    juce::File rootContainerDirectory;
    juce::String displayName;
    R2CloudStorageProvider::Status status = R2CloudStorageProvider::Status::NotAuthenticated;

    juce::File getICloudContainerURLForIdentifier(const juce::String& containerIdentifier) {
        NSString* identifier = nil;
        if (containerIdentifier.isNotEmpty())
            identifier = (NSString*)containerIdentifier.toCFString();

        NSURL* containerURL = [[NSFileManager defaultManager] URLForUbiquityContainerIdentifier:identifier];
        if (containerURL != nil) {
            NSURL* documentsURL = [containerURL URLByAppendingPathComponent:@"Documents"];
            NSError* error = nil;
            if (![[NSFileManager defaultManager] fileExistsAtPath:[documentsURL path]]) {
                [[NSFileManager defaultManager] createDirectoryAtURL:documentsURL withIntermediateDirectories:true attributes:nil error:&error];
            }
            if (error == nil)
                return juce::File(juce::String([[documentsURL path] UTF8String]));
        }
        return {};
    }
    
    juce::File getFileFromId(const juce::String& fileId) const {
        if (fileId.isEmpty() || fileId == "root") return rootContainerDirectory;
        juce::MemoryOutputStream decoded;
        if (juce::Base64::convertFromBase64(decoded, fileId)) {
            return rootContainerDirectory.getChildFile(decoded.toString());
        }
        return {};
    }

    juce::String getIdFromFile(const juce::File& file) const {
        if (!file.exists() || !rootContainerDirectory.exists() || file == rootContainerDirectory) return {};
        return juce::Base64::toBase64(file.getRelativePathFrom(rootContainerDirectory));
    }
};

//==============================================================================
// R2IcloudDriveProvider implementation
//==============================================================================

R2IcloudDriveProvider::R2IcloudDriveProvider(const juce::String& containerId)
{
    pimpl = std::make_unique<IcloudImpl>(containerId);
}

R2IcloudDriveProvider::~R2IcloudDriveProvider() = default;

// The public methods now simply forward the call to the synchronous pimpl methods.
// The pimpl methods will post the callback to the message thread if one is provided.
void R2IcloudDriveProvider::authenticate(AuthCallback c) { if (pimpl) pimpl->authenticate(c); else if (c) c(false, "pimpl not initialized"); }
void R2IcloudDriveProvider::signOut() { if (pimpl) pimpl->signOut(); }
R2CloudStorageProvider::Status R2IcloudDriveProvider::getAuthStatus() const { return pimpl ? pimpl->getStatus() : Status::NotAuthenticated; }
juce::String R2IcloudDriveProvider::getDisplayName() const { return pimpl ? pimpl->getDisplayName() : "iCloud Drive (Uninitialized)"; }
R2CloudStorageProvider::ServiceType R2IcloudDriveProvider::getServiceType() const { return ServiceType::iCloudDrive; }
void R2IcloudDriveProvider::uploadFileByPath(const juce::String& f, const juce::MemoryBlock& d, FileOperationCallback c) { if (pimpl) pimpl->uploadFileByPath(f, d, c); else if (c) c(false, "pimpl not initialized"); }
void R2IcloudDriveProvider::downloadFileByPath(const juce::String& f, DownloadCallback c) { if (pimpl) pimpl->downloadFileByPath(f, c); else if (c) c(false, {}, "pimpl not initialized"); }
void R2IcloudDriveProvider::listFiles(const juce::String& f, FileListCallback c) { if (pimpl) pimpl->listFiles(f, c); else if (c) c(false, {}, "pimpl not initialized"); }
void R2IcloudDriveProvider::uploadFile(const juce::String& f, const juce::MemoryBlock& d, const juce::String& p, FileOperationCallback c) { if (pimpl) pimpl->uploadFile(f, d, p, c); else if (c) c(false, "pimpl not initialized"); }
void R2IcloudDriveProvider::downloadFile(const juce::String& f, DownloadCallback c) { if (pimpl) pimpl->downloadFile(f, c); else if (c) c(false, {}, "pimpl not initialized"); }
void R2IcloudDriveProvider::deleteFile(const juce::String& f, FileOperationCallback c) { if (pimpl) pimpl->deleteFile(f, c); else if (c) c(false, "pimpl not initialized"); }
void R2IcloudDriveProvider::createFolder(const juce::String& f, const juce::String& p, FileOperationCallback c) { if (pimpl) pimpl->createFolder(f, p, c); else if (c) c(false, "pimpl not initialized"); }

} // namespace r2juce

// #endif // JUCE_MAC || JUCE_IOS

