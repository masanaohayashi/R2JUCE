#pragma once

#include "R2CloudStorageProvider.h"

namespace r2juce {

/**
    @brief A concrete implementation of R2CloudStorageProvider that uses the local file system for storage.
    @details This class simulates the behavior of a cloud storage service by performing
             file operations within a specified root directory on the local disk. It is
             always in an 'authenticated' state. File and folder IDs are represented
             as Base64-encoded relative paths from the root directory.
*/
class R2LocalStorageProvider : public R2CloudStorageProvider
{
public:
    R2LocalStorageProvider();
    
    //==============================================================================
    // R2CloudStorageProvider interface
    
    /** @brief Always succeeds immediately, as local storage requires no authentication. (Inherited) */
    void authenticate(AuthCallback callback) override;
    
    /** @brief Performs no action for local storage. (Inherited) */
    void signOut() override;

    /** @brief Always returns Status::Authenticated. (Inherited) */
    Status getAuthStatus() const override;

    /** @brief Returns "Local Storage" along with the root path. (Inherited) */
    juce::String getDisplayName() const override;

    /** @brief Saves data to a file at the specified relative path from the root directory. (Inherited) */
    void uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback) override;
    
    /** @brief Loads data from a file at the specified relative path from the root directory. (Inherited) */
    void downloadFileByPath(const juce::String& filePath, DownloadCallback callback) override;
    
    /** @brief Lists files and folders within a directory specified by its ID. (Inherited) */
    void listFiles(const juce::String& folderId, FileListCallback callback) override;
    
    /** @brief Saves data as a new file within a directory specified by its ID. (Inherited) */
    void uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                    const juce::String& folderId, FileOperationCallback callback) override;
                    
    /** @brief Loads data from a file specified by its ID. (Inherited) */
    void downloadFile(const juce::String& fileId, DownloadCallback callback) override;
    
    /** @brief Deletes a file or folder specified by its ID. (Inherited) */
    void deleteFile(const juce::String& fileId, FileOperationCallback callback) override;
    
    /** @brief Creates a new folder within a parent directory specified by its ID. (Inherited) */
    void createFolder(const juce::String& folderName, const juce::String& parentId,
                      FileOperationCallback callback) override;
                      
    //==============================================================================
    /** @brief Sets the root directory for all local storage operations.
        @param directory The juce::File object representing the new root directory.
                     The directory will be created if it doesn't exist.
    */
    void setRootDirectory(const juce::File& directory);
    
    /** @brief Gets the current root directory.
        @return A juce::File object for the root directory.
    */
    juce::File getRootDirectory() const;
    
    /** @brief Converts a Base64-encoded ID string back into a juce::File object.
        @param fileId The ID to convert. "root" or an empty string maps to the root directory.
        @return The corresponding juce::File.
    */
    juce::File getFileFromId(const juce::String& fileId) const;

    /** @brief Converts a juce::File object into a Base64-encoded ID string representing its relative path.
        @param file The file to convert.
        @return The Base64-encoded ID string.
    */
    juce::String getIdFromFile(const juce::File& file) const;

private:
    juce::File rootDirectory;
};

} // namespace r2juce
