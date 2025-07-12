#include "R2LocalStorageProvider.h"

namespace r2juce {

R2LocalStorageProvider::R2LocalStorageProvider(const juce::String& appGroupId)
    : appGroupId(appGroupId) {
    currentStatus = Status::Authenticated;
}

void R2LocalStorageProvider::authenticate(AuthCallback callback) {
    if (callback) {
        // Local storage does not require authentication, so we succeed immediately.
        callback(true, "");
    }
}

void R2LocalStorageProvider::signOut() {
    // Nothing to do for local storage.
}

R2CloudStorageProvider::Status R2LocalStorageProvider::getAuthStatus() const {
    return Status::Authenticated;
}

juce::String R2LocalStorageProvider::getDisplayName() const { return "Local"; }

R2CloudStorageProvider::ServiceType R2LocalStorageProvider::getServiceType()
    const {
    return ServiceType::Local;
}

juce::File R2LocalStorageProvider::getLocalStorageRoot() const {
#if JUCE_MAC
    if (appGroupId.isNotEmpty()) {
        auto container =
            juce::File::getContainerForSecurityApplicationGroupIdentifier(
                appGroupId);
        
        // Add detailed logging for diagnostics
        DBG("--- App Group Container Access Log ---");
        DBG("Requested App Group ID: " + appGroupId);
        DBG("Returned container path: " + container.getFullPathName());
        DBG("container.isDirectory(): " + juce::String(container.isDirectory() ? "true" : "false"));

        if (container.isDirectory()) {
            DBG("Successfully accessed App Group container.");
            return container;
        }
        
        DBG("Failed to access App Group container, falling back to local directory.");
    } else {
        DBG("App Group ID is empty, using local directory.");
    }
    
    // If the container is not available for any reason, fall back to a standard
    // app data directory to ensure functionality.
    return juce::File::getSpecialLocation(
               juce::File::userApplicationDataDirectory)
        .getChildFile("CloudDocLocal");
#else
    // On other platforms (Windows, Linux), use a dedicated folder within the
    // standard user application data directory.
    return juce::File::getSpecialLocation(
               juce::File::userApplicationDataDirectory)
        .getChildFile("CloudDocLocal");
#endif
}

void R2LocalStorageProvider::uploadFileByPath(
    const juce::String& filePath, const juce::MemoryBlock& data,
    FileOperationCallback callback) {
    auto rootDir = getLocalStorageRoot();
    if (!rootDir.exists()) {
        if (!rootDir.createDirectory()) {
            if (callback) callback(false, "Failed to create local storage directory.");
            return;
        }
    }

    juce::File fileToSave = rootDir.getChildFile(filePath);
    DBG("Local Storage: Writing to " + fileToSave.getFullPathName());

    // Ensure parent directory exists before attempting to write the file.
    auto parentDir = fileToSave.getParentDirectory();
    if (!parentDir.exists()) {
        if (!parentDir.createDirectory()) {
            if (callback)
                callback(false,
                         "Could not create directory: " + parentDir.getFullPathName());
            return;
        }
    }

    if (fileToSave.replaceWithData(data.getData(), data.getSize())) {
        if (callback) callback(true, "");
    } else {
        if (callback) callback(false, "Failed to write to local file.");
    }
}

void R2LocalStorageProvider::downloadFileByPath(const juce::String& filePath,
                                                DownloadCallback callback) {
    auto rootDir = getLocalStorageRoot();
    juce::File fileToLoad = rootDir.getChildFile(filePath);
    DBG("Local Storage: Reading from " + fileToLoad.getFullPathName());

    if (!fileToLoad.existsAsFile()) {
        if (callback) callback(false, {}, "File not found.");
        return;
    }

    juce::MemoryBlock data;
    if (fileToLoad.loadFileAsData(data)) {
        if (callback) callback(true, data, "");
    } else {
        if (callback) callback(false, {}, "Failed to read from local file.");
    }
}

void R2LocalStorageProvider::listFiles(const juce::String& folderId,
                                       FileListCallback callback) {
    // This operation is not implemented for this example.
    if (callback) {
        callback(false, {}, "Not implemented");
    }
}

void R2LocalStorageProvider::uploadFile(const juce::String& fileName,
                                        const juce::MemoryBlock& data,
                                        const juce::String& folderId,
                                        FileOperationCallback callback) {
    // For local storage, construct the path from folderId and fileName.
    juce::String fullPath;
    if (folderId.isNotEmpty()) {
        if (folderId.endsWith("/") || folderId.endsWith("\\"))
            fullPath = folderId + fileName;
        else
            fullPath = folderId + "/" + fileName;
    } else {
        fullPath = fileName;
    }
    uploadFileByPath(fullPath, data, callback);
}

void R2LocalStorageProvider::downloadFile(const juce::String& fileId,
                                          DownloadCallback callback) {
    // For local storage, the file ID is simply the file path.
    downloadFileByPath(fileId, callback);
}

void R2LocalStorageProvider::deleteFile(const juce::String& fileId,
                                        FileOperationCallback callback) {
    // This operation is not implemented for this example.
    if (callback) {
        callback(false, "Not implemented");
    }
}

void R2LocalStorageProvider::createFolder(const juce::String& folderName,
                                          const juce::String& parentId,
                                          FileOperationCallback callback) {
    // This operation is not implemented for this example.
    if (callback) {
        callback(false, "Not implemented");
    }
}

}  // namespace r2juce

