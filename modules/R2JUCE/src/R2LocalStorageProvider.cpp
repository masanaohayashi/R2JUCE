#include "R2LocalStorageProvider.h"

namespace r2juce {

R2LocalStorageProvider::R2LocalStorageProvider()
{
    // デフォルトのルートディレクトリを設定
    auto documentsDir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
    rootDirectory = documentsDir.getChildFile("CloudDoc");
    rootDirectory.createDirectory();
    
    currentStatus = Status::Authenticated;
}

void R2LocalStorageProvider::authenticate(AuthCallback callback)
{
    // ローカルストレージは認証不要
    currentStatus = Status::Authenticated;
    if (callback)
        callback(true, "");
}

void R2LocalStorageProvider::signOut()
{
    // ローカルストレージはサインアウト不要
}

R2CloudStorageProvider::Status R2LocalStorageProvider::getAuthStatus() const
{
    return Status::Authenticated;
}

juce::String R2LocalStorageProvider::getDisplayName() const
{
    return "Local Storage (" + rootDirectory.getFullPathName() + ")";
}

void R2LocalStorageProvider::setRootDirectory(const juce::File& directory)
{
    rootDirectory = directory;
    rootDirectory.createDirectory();
}

juce::File R2LocalStorageProvider::getRootDirectory() const
{
    return rootDirectory;
}

juce::File R2LocalStorageProvider::getFileFromId(const juce::String& fileId) const
{
    if (fileId.isEmpty() || fileId == "root")
        return rootDirectory;
    
    // Base64デコードしてパスを復元
    juce::MemoryOutputStream decoded;
    if (juce::Base64::convertFromBase64(decoded, fileId))
    {
        juce::String relativePath = decoded.toString();
        return rootDirectory.getChildFile(relativePath);
    }
    
    return {};
}

juce::String R2LocalStorageProvider::getIdFromFile(const juce::File& file) const
{
    if (!file.exists())
        return {};
    
    if (file == rootDirectory)
        return "root";
    
    // rootDirectoryからの相対パスをBase64エンコード
    juce::String relativePath = file.getRelativePathFrom(rootDirectory);
    return juce::Base64::toBase64(relativePath);
}

void R2LocalStorageProvider::listFiles(const juce::String& folderId, FileListCallback callback)
{
    juce::Thread::launch([this, folderId, callback]()
    {
        juce::Array<FileInfo> files;
        
        auto folder = getFileFromId(folderId);
        if (!folder.exists() || !folder.isDirectory())
        {
            juce::MessageManager::callAsync([callback]()
            {
                callback(false, {}, "Folder not found");
            });
            return;
        }
        
        auto children = folder.findChildFiles(juce::File::findFilesAndDirectories, false);
        
        for (const auto& child : children)
        {
            FileInfo info;
            info.name = child.getFileName();
            info.id = getIdFromFile(child);
            info.isFolder = child.isDirectory();
            info.size = child.getSize();
            info.modifiedTime = child.getLastModificationTime();
            
            if (!info.isFolder)
            {
                info.mimeType = "application/octet-stream";
                // 簡単なMIMEタイプ判定
                auto extension = child.getFileExtension().toLowerCase();
                if (extension == ".txt") info.mimeType = "text/plain";
                else if (extension == ".json") info.mimeType = "application/json";
                else if (extension == ".xml") info.mimeType = "application/xml";
            }
            
            files.add(info);
        }
        
        juce::MessageManager::callAsync([callback, files]()
        {
            callback(true, files, "");
        });
    });
}

void R2LocalStorageProvider::uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                                       const juce::String& folderId, FileOperationCallback callback)
{
    juce::Thread::launch([this, fileName, data, folderId, callback]()
    {
        auto folder = getFileFromId(folderId);
        if (!folder.exists())
            folder.createDirectory();
        
        auto targetFile = folder.getChildFile(fileName);
        
        bool success = targetFile.replaceWithData(data.getData(), data.getSize());
        
        juce::MessageManager::callAsync([callback, success]()
        {
            callback(success, success ? "" : "Failed to write file");
        });
    });
}

void R2LocalStorageProvider::downloadFile(const juce::String& fileId, DownloadCallback callback)
{
    juce::Thread::launch([this, fileId, callback]()
    {
        auto file = getFileFromId(fileId);
        
        if (!file.exists() || file.isDirectory())
        {
            juce::MessageManager::callAsync([callback]()
            {
                callback(false, {}, "File not found");
            });
            return;
        }
        
        juce::MemoryBlock data;
        bool success = file.loadFileAsData(data);
        
        juce::MessageManager::callAsync([callback, success, data]()
        {
            callback(success, data, success ? "" : "Failed to read file");
        });
    });
}

void R2LocalStorageProvider::deleteFile(const juce::String& fileId, FileOperationCallback callback)
{
    juce::Thread::launch([this, fileId, callback]()
    {
        auto file = getFileFromId(fileId);
        
        bool success = file.exists() && file.deleteRecursively();
        
        juce::MessageManager::callAsync([callback, success]()
        {
            callback(success, success ? "" : "Failed to delete file");
        });
    });
}

void R2LocalStorageProvider::createFolder(const juce::String& folderName, const juce::String& parentId,
                                         FileOperationCallback callback)
{
    juce::Thread::launch([this, folderName, parentId, callback]()
    {
        auto parent = getFileFromId(parentId);
        auto newFolder = parent.getChildFile(folderName);
        
        bool success = newFolder.createDirectory();
        
        juce::MessageManager::callAsync([callback, success]()
        {
            callback(success, success ? "" : "Failed to create folder");
        });
    });
}

} // namespace r2juce
