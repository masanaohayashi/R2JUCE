#include "R2GoogleDriveProvider.h"

namespace r2juce {

R2GoogleDriveProvider::R2GoogleDriveProvider(const juce::String& newClientId, const juce::String& newClientSecret)
    : clientId(newClientId), clientSecret(newClientSecret)
{
    currentStatus = Status::NotAuthenticated;
    loadTokens();
}

R2CloudStorageProvider::ServiceType R2GoogleDriveProvider::getServiceType() const
{
    return ServiceType::GoogleDrive;
}

void R2GoogleDriveProvider::setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken)
{
    accessToken = newAccessToken;
    if (newRefreshToken.isNotEmpty())
    {
        refreshToken = newRefreshToken;
    }
    tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(3500);
    currentStatus = Status::Authenticated;
    saveTokens();
}

void R2GoogleDriveProvider::authenticate(AuthCallback callback)
{
    if (clientId.isEmpty() || clientSecret.isEmpty())
    {
        if (callback) juce::MessageManager::callAsync([callback] { callback(false, "Client credentials not set."); });
        return;
    }

    if (isTokenValid())
    {
        currentStatus = Status::Authenticated;
        if (callback) juce::MessageManager::callAsync([callback] { callback(true, "Already authenticated."); });
        return;
    }

    if (refreshToken.isNotEmpty())
    {
        currentStatus = Status::Authenticating;
        refreshAccessToken([this, callback](bool success)
        {
            if (success)
            {
                currentStatus = Status::Authenticated;
                if (callback) juce::MessageManager::callAsync([callback] { callback(true, "Authentication refreshed successfully."); });
            }
            else
            {
                currentStatus = Status::NotAuthenticated;
                if (callback) juce::MessageManager::callAsync([callback] { callback(false, "device flow authentication required"); });
            }
        });
        return;
    }
    
    currentStatus = Status::NotAuthenticated;
    if (callback) juce::MessageManager::callAsync([callback] { callback(false, "device flow authentication required"); });
}

void R2GoogleDriveProvider::signOut()
{
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = {};
    currentStatus = Status::NotAuthenticated;
    
    auto tokenFile = getTokenFile();
    if (tokenFile.exists())
        tokenFile.deleteFile();
}

R2CloudStorageProvider::Status R2GoogleDriveProvider::getAuthStatus() const { return currentStatus; }
juce::String R2GoogleDriveProvider::getDisplayName() const { return "Google Drive"; }
bool R2GoogleDriveProvider::isTokenValid() const { return accessToken.isNotEmpty() && juce::Time::getCurrentTime() < tokenExpiry; }

void R2GoogleDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    if (refreshToken.isEmpty())
    {
        if (callback) juce::MessageManager::callAsync([callback] { callback(false); });
        return;
    }

    auto self = shared_from_this();

    juce::Thread::launch([this, self, callback]()
    {
        juce::String postData = "client_id=" + juce::URL::addEscapeChars(clientId, false)
                              + "&client_secret=" + juce::URL::addEscapeChars(clientSecret, false)
                              + "&refresh_token=" + juce::URL::addEscapeChars(refreshToken, false)
                              + "&grant_type=refresh_token";

        juce::URL url("https://oauth2.googleapis.com/token");
        url = url.withPOSTData(postData);

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                       .withExtraHeaders("Content-Type: application/x-www-form-urlencoded")
                       .withConnectionTimeoutMs(30000);

        if (auto stream = url.createInputStream(options))
        {
            auto response = stream->readEntireStreamAsString();
            parseTokenResponse(response, [callback](bool parsedSuccess, const juce::String&)
            {
                 juce::MessageManager::callAsync([callback, parsedSuccess](){
                    if (callback) callback(parsedSuccess);
                 });
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback](){
                if (callback) callback(false);
            });
        }
    });
}

void R2GoogleDriveProvider::parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> onComplete)
{
    auto json = juce::JSON::parse(response);
    if (auto* obj = json.getDynamicObject())
    {
        if (obj->hasProperty("access_token"))
        {
            accessToken = obj->getProperty("access_token").toString();
            
            if (obj->hasProperty("refresh_token"))
                refreshToken = obj->getProperty("refresh_token").toString();
            
            if (obj->hasProperty("expires_in"))
                tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds((int)obj->getProperty("expires_in") - 100);

            currentStatus = Status::Authenticated;
            saveTokens();
            if (onComplete) onComplete(true, "");
            return;
        }
        else if (obj->hasProperty("error"))
        {
            auto error = obj->getProperty("error").toString();
            auto errorDesc = obj->getProperty("error_description").toString();
            currentStatus = Status::Error;

            if (error == "invalid_grant")
            {
                signOut();
            }

            if (onComplete) onComplete(false, error + ": " + errorDesc);
            return;
        }
    }
    
    currentStatus = Status::Error;
    if (onComplete) onComplete(false, "Failed to parse token response.");
}


void R2GoogleDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& httpMethod, const juce::StringPairArray& headers, const juce::MemoryBlock& postData, std::function<void(bool, juce::String)> callback)
{
    auto self = shared_from_this();

    if (!isTokenValid())
    {
        refreshAccessToken([this, self, endpoint, httpMethod, headers, postData, callback](bool success)
        {
            if (success)
                makeAPIRequest(endpoint, httpMethod, headers, postData, callback);
            else
                juce::MessageManager::callAsync([callback] { if (callback) callback(false, "Authentication required."); });
        });
        return;
    }

    juce::Thread::launch([this, self, endpoint, httpMethod, headers, postData, callback]()
    {
        juce::URL url(endpoint);

        juce::StringPairArray finalHeaders = headers;
        finalHeaders.set("Authorization", "Bearer " + accessToken);

        juce::String headerString;
        for (int i = 0; i < finalHeaders.size(); ++i)
            headerString << finalHeaders.getAllKeys()[i] << ": " << finalHeaders.getAllValues()[i] << "\r\n";

        if ((httpMethod == "POST" || httpMethod == "PATCH") && postData.getSize() > 0)
        {
            url = url.withPOSTData(postData);
        }

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withConnectionTimeoutMs(30000)
                       .withHttpRequestCmd(httpMethod)
                       .withExtraHeaders(headerString);

        if (auto stream = url.createInputStream(options))
        {
            auto response = stream->readEntireStreamAsString();
            juce::MessageManager::callAsync([callback, response] {
                if (callback) callback(true, response);
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback] {
                if (callback) callback(false, "Failed to connect to server.");
            });
        }
    });
}

void R2GoogleDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& httpMethod, const juce::StringPairArray& headers, const juce::String& postData, std::function<void(bool, juce::String)> callback)
{
    makeAPIRequest(endpoint, httpMethod, headers, juce::MemoryBlock(postData.toRawUTF8(), postData.getNumBytesAsUTF8()), callback);
}

void R2GoogleDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    uploadFile(filePath, data, "path", callback);
}

void R2GoogleDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    auto pathParts = juce::StringArray::fromTokens(filePath, "/", "");
    pathParts.removeEmptyStrings();
    if (pathParts.isEmpty())
    {
        if (callback) juce::MessageManager::callAsync([callback] { callback(false, {}, "Invalid file path"); });
        return;
    }
    findFileByPath(pathParts, "root", 0, callback);
}

void R2GoogleDriveProvider::findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId, int pathIndex, DownloadCallback callback)
{
    if (pathIndex >= pathParts.size())
    {
        if (callback) juce::MessageManager::callAsync([callback] { callback(false, {}, "Invalid path index."); });
        return;
    }
    
    auto targetName = pathParts[pathIndex];
    bool isLastPart = (pathIndex == pathParts.size() - 1);
    
    listFiles(currentFolderId, [=](bool success, juce::Array<FileInfo> files, juce::String errorMessage)
    {
        if (!success)
        {
            if (callback) callback(false, {}, "Failed to list files: " + errorMessage);
            return;
        }
        
        for (const auto& file : files)
        {
            if (file.name == targetName)
            {
                if (isLastPart && !file.isFolder)
                {
                    downloadFile(file.id, callback);
                    return;
                }
                if (!isLastPart && file.isFolder)
                {
                    findFileByPath(pathParts, file.id, pathIndex + 1, callback);
                    return;
                }
            }
        }
        
        if (callback) callback(false, {}, "File or folder not found: " + targetName);
    });
}


void R2GoogleDriveProvider::uploadFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback)
{
    if (folderId == "path")
    {
        auto pathParts = juce::StringArray::fromTokens(fileName, "/", "");
        pathParts.removeEmptyStrings();
        if (pathParts.size() <= 1)
        {
            uploadToFolder(fileName, data, "root", callback);
        }
        else
        {
            auto actualFileName = pathParts[pathParts.size() - 1];
            pathParts.remove(pathParts.size() - 1);

            createFolderPath(pathParts, "root", 0, [this, actualFileName, data, callback](bool success, juce::String newFolderId, juce::String errorMessage)
            {
                if (success)
                    uploadToFolder(actualFileName, data, newFolderId, callback);
                else if (callback)
                    juce::MessageManager::callAsync([callback, errorMessage] { callback(false, "Failed to create folder path: " + errorMessage); });
            });
        }
    }
    else
    {
        uploadToFolder(fileName, data, folderId, callback);
    }
}

void R2GoogleDriveProvider::uploadToFolder(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback)
{
    juce::String targetFolderId = folderId.isEmpty() ? "root" : folderId;
    juce::String query = "'" + targetFolderId + "' in parents and name = '" + fileName + "' and mimeType != 'application/vnd.google-apps.folder' and trashed = false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false) + "&fields=files(id)";

    makeAPIRequest(endpoint, "GET", {}, "", [=](bool success, const juce::String& response)
    {
        if (!success)
        {
            if (callback) callback(false, "API connection failed while checking for existing file.");
            return;
        }

        juce::String existingFileId;
        auto json = juce::JSON::parse(response);
        if (auto* obj = json.getDynamicObject())
            if (auto* filesArray = obj->getProperty("files").getArray())
                if (!filesArray->isEmpty())
                    if (auto* fileObj = filesArray->getUnchecked(0).getDynamicObject())
                        existingFileId = fileObj->getProperty("id").toString();
        
        if (existingFileId.isNotEmpty())
            updateExistingFile(existingFileId, data, callback);
        else
            uploadNewFile(fileName, data, targetFolderId, callback);
    });
}

void R2GoogleDriveProvider::updateExistingFile(const juce::String& fileId, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    juce::String endpoint = "https://www.googleapis.com/upload/drive/v3/files/" + fileId + "?uploadType=media";
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/octet-stream");
    
    makeAPIRequest(endpoint, "PATCH", headers, data, [callback](bool success, const juce::String& response)
    {
        if (!success) { if (callback) callback(false, "Update failed: " + response); return; }

        auto json = juce::JSON::parse(response);
        if (json.isObject() && json.getDynamicObject()->hasProperty("id"))
        {
            if (callback) callback(true, "File updated successfully");
        }
        else
        {
            if (callback) callback(false, "Update failed: " + response);
        }
    });
}

void R2GoogleDriveProvider::uploadNewFile(const juce::String& fileName, const juce::MemoryBlock& data, const juce::String& folderId, FileOperationCallback callback)
{
    juce::String boundary = "---r2juce-boundary-" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64());
    
    juce::DynamicObject::Ptr metadataObject = new juce::DynamicObject();
    metadataObject->setProperty("name", fileName);
    if (folderId.isNotEmpty())
        metadataObject->setProperty("parents", juce::var(juce::StringArray(folderId)));
    
    juce::String metadataJson = juce::JSON::toString(juce::var(metadataObject.get()));
    
    juce::MemoryBlock multipartBody;
    juce::String part1 = "--" + boundary + "\r\n";
    multipartBody.append(part1.toRawUTF8(), part1.getNumBytesAsUTF8());
    juce::String part2 = "Content-Type: application/json; charset=UTF-8\r\n\r\n";
    multipartBody.append(part2.toRawUTF8(), part2.getNumBytesAsUTF8());
    multipartBody.append(metadataJson.toRawUTF8(), metadataJson.getNumBytesAsUTF8());
    juce::String part3 = "\r\n--" + boundary + "\r\n";
    multipartBody.append(part3.toRawUTF8(), part3.getNumBytesAsUTF8());
    juce::String part4 = "Content-Type: application/octet-stream\r\n\r\n";
    multipartBody.append(part4.toRawUTF8(), part4.getNumBytesAsUTF8());
    multipartBody.append(data.getData(), data.getSize());
    juce::String part5 = "\r\n--" + boundary + "--\r\n";
    multipartBody.append(part5.toRawUTF8(), part5.getNumBytesAsUTF8());

    juce::String endpoint = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "multipart/related; boundary=" + boundary);
    
    makeAPIRequest(endpoint, "POST", headers, multipartBody, [callback](bool success, const juce::String& response)
    {
        if (!success) { if (callback) callback(false, "Upload failed: " + response); return; }

        auto json = juce::JSON::parse(response);
        if (json.isObject() && json.getDynamicObject()->hasProperty("id"))
        {
             if (callback) callback(true, "File uploaded successfully");
        }
        else
        {
             if (callback) callback(false, "Upload failed: " + response);
        }
    });
}

void R2GoogleDriveProvider::createFolderPath(const juce::StringArray& folderPath, const juce::String& parentFolderId, int pathIndex, std::function<void(bool, juce::String, juce::String)> callback)
{
    if (pathIndex >= folderPath.size())
    {
        if (callback) juce::MessageManager::callAsync([=] { callback(true, parentFolderId, ""); });
        return;
    }

    auto folderName = folderPath[pathIndex];
    juce::String query = "'" + parentFolderId + "' in parents and name = '" + folderName + "' and mimeType = 'application/vnd.google-apps.folder' and trashed = false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false) + "&fields=files(id)";
    
    makeAPIRequest(endpoint, "GET", {}, "", [=](bool success, const juce::String& response)
    {
        if (!success)
        {
            if (callback) callback(false, "", "API connection failed while checking for folder.");
            return;
        }

        juce::String existingFolderId;
        auto json = juce::JSON::parse(response);
        if (auto* obj = json.getDynamicObject())
            if (auto* filesArray = obj->getProperty("files").getArray())
                if (!filesArray->isEmpty())
                    if (auto* fileObj = filesArray->getUnchecked(0).getDynamicObject())
                        existingFolderId = fileObj->getProperty("id").toString();

        if (existingFolderId.isNotEmpty())
        {
            createFolderPath(folderPath, existingFolderId, pathIndex + 1, callback);
        }
        else
        {
            juce::DynamicObject::Ptr newFolderMeta = new juce::DynamicObject();
            newFolderMeta->setProperty("name", folderName);
            newFolderMeta->setProperty("mimeType", "application/vnd.google-apps.folder");
            newFolderMeta->setProperty("parents", juce::var(juce::StringArray(parentFolderId)));
            
            juce::String metadata = juce::JSON::toString(juce::var(newFolderMeta.get()));
            juce::StringPairArray createHeaders;
            createHeaders.set("Content-Type", "application/json");

            makeAPIRequest("https://www.googleapis.com/drive/v3/files", "POST", createHeaders, metadata, [=](bool createSuccess, const juce::String& createResponse)
            {
                if (!createSuccess)
                {
                    if (callback) callback(false, "", "Failed to create folder: " + createResponse);
                    return;
                }
                
                juce::String newFolderId;
                auto createJson = juce::JSON::parse(createResponse);
                if (auto* createObj = createJson.getDynamicObject())
                    newFolderId = createObj->getProperty("id").toString();

                if (newFolderId.isNotEmpty())
                    createFolderPath(folderPath, newFolderId, pathIndex + 1, callback);
                else
                    if (callback) callback(false, "", "Failed to parse created folder ID.");
            });
        }
    });
}

void R2GoogleDriveProvider::listFiles(const juce::String& folderId, FileListCallback callback)
{
    juce::String query = "'" + (folderId.isEmpty() ? "root" : folderId) + "' in parents and trashed=false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false) + "&fields=files(id,name,mimeType,modifiedTime,size)";
    
    makeAPIRequest(endpoint, "GET", {}, "", [callback](bool success, const juce::String& response)
    {
        if (!success)
        {
            if (callback) callback(false, {}, "API request to list files failed: " + response);
            return;
        }

        juce::Array<FileInfo> fileList;
        auto json = juce::JSON::parse(response);
        if (auto* obj = json.getDynamicObject())
        {
            if (auto* files = obj->getProperty("files").getArray())
            {
                for (const auto& fileVar : *files)
                {
                    if (auto* fileObj = fileVar.getDynamicObject())
                    {
                        FileInfo info;
                        info.id = fileObj->getProperty("id").toString();
                        info.name = fileObj->getProperty("name").toString();
                        info.mimeType = fileObj->getProperty("mimeType").toString();
                        info.isFolder = (info.mimeType == "application/vnd.google-apps.folder");
                        info.modifiedTime = juce::Time::fromISO8601(fileObj->getProperty("modifiedTime").toString());
                        if (fileObj->hasProperty("size"))
                            info.size = fileObj->getProperty("size").toString().getLargeIntValue();
                        
                        fileList.add(info);
                    }
                }
                if (callback) callback(true, fileList, "");
                return;
            }
        }
        if (callback) callback(false, {}, "Failed to parse list files response.");
    });
}

void R2GoogleDriveProvider::downloadFile(const juce::String& fileId, DownloadCallback callback)
{
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files/" + fileId + "?alt=media";
    
    auto requestLogic = [=]()
    {
        juce::Thread::launch([=]()
        {
            juce::URL url(endpoint);
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                               .withConnectionTimeoutMs(60000)
                               .withExtraHeaders("Authorization: Bearer " + accessToken);

            if (auto stream = url.createInputStream(options))
            {
                juce::MemoryBlock data;
                stream->readIntoMemoryBlock(data);
                
                juce::MessageManager::callAsync([=] {
                    if (callback) callback(true, data, "");
                });
            }
            else
            {
                juce::MessageManager::callAsync([=] {
                    if (callback) callback(false, {}, "Failed to create download stream.");
                });
            }
        });
    };

     if (!isTokenValid())
    {
        refreshAccessToken([this, requestLogic, callback](bool success) {
            if (success)
                requestLogic();
            else
                juce::MessageManager::callAsync([=] { if (callback) callback(false, {}, "Authentication required."); });
        });
    }
    else
    {
        requestLogic();
    }
}

void R2GoogleDriveProvider::deleteFile(const juce::String& fileId, FileOperationCallback callback)
{
    makeAPIRequest("https://www.googleapis.com/drive/v3/files/" + fileId, "DELETE", {}, "", [callback](bool success, const juce::String& response)
    {
        if (success && response.isEmpty())
        {
            if (callback) callback(true, "File deleted.");
        }
        else
        {
            if (callback) callback(false, "Delete failed: " + response);
        }
    });
}

void R2GoogleDriveProvider::createFolder(const juce::String& folderName, const juce::String& parentId, FileOperationCallback callback)
{
    createFolderPath({folderName}, parentId.isEmpty() ? "root" : parentId, 0, [callback](bool success, const juce::String&, const juce::String& errorMessage)
    {
        if (callback) callback(success, errorMessage);
    });
}

bool R2GoogleDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
    if (!tokenFile.existsAsFile())
        return false;

    auto parseResult = juce::JSON::parse(tokenFile);
    if (auto* obj = parseResult.getDynamicObject())
    {
        accessToken = obj->getProperty("access_token").toString();
        refreshToken = obj->getProperty("refresh_token").toString();
        tokenExpiry = juce::Time(obj->getProperty("token_expiry").toString().getLargeIntValue());
        
        if (isTokenValid())
        {
            currentStatus = Status::Authenticated;
        }
        else
        {
            currentStatus = Status::NotAuthenticated;
        }
        return true;
    }
    return false;
}

void R2GoogleDriveProvider::saveTokens()
{
    auto tokenFile = getTokenFile();
    if (!tokenFile.getParentDirectory().exists())
        tokenFile.getParentDirectory().createDirectory();

    juce::DynamicObject::Ptr tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", tokenExpiry.toMilliseconds());
    
    tokenFile.replaceWithText(juce::JSON::toString(juce::var(tokenData.get()), true));
}


juce::File R2GoogleDriveProvider::getTokenFile() const
{
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
           .getChildFile("R2JUCE_CloudApp")
           .getChildFile("google_drive_tokens.json");
}

} // namespace r2juce

