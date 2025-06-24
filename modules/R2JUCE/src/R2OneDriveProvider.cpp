#include "R2OneDriveProvider.h"

namespace r2juce {

R2OneDriveProvider::R2OneDriveProvider()
{
    currentStatus = Status::NotAuthenticated;
    loadTokens();
}

void R2OneDriveProvider::setClientCredentials(const juce::String& cid, const juce::String& csecret)
{
    clientId = cid;
    clientSecret = csecret;
}

void R2OneDriveProvider::authenticate(AuthCallback callback)
{
    if (clientId.isEmpty())
    {
        if (callback) callback(false, "Client ID not set");
        return;
    }
    
    if (isTokenValid())
    {
        currentStatus = Status::Authenticated;
        if (callback) callback(true, "Already authenticated");
        return;
    }
    
    if (refreshToken.isNotEmpty())
    {
        currentStatus = Status::Authenticating;
        refreshAccessToken([this, callback](bool success)
        {
            currentStatus = success ? Status::Authenticated : Status::NotAuthenticated;
            juce::MessageManager::callAsync([callback, success](){
                if (callback) callback(success, success ? "Refreshed token" : "Refresh failed, new auth required");
            });
        });
        return;
    }
    
    currentStatus = Status::NotAuthenticated;
    if (callback) callback(false, "device flow authentication required");
}

void R2OneDriveProvider::signOut()
{
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = juce::Time();
    currentStatus = Status::NotAuthenticated;
    if (auto tokenFile = getTokenFile(); tokenFile.exists())
        tokenFile.deleteFile();
}

R2CloudStorageProvider::Status R2OneDriveProvider::getAuthStatus() const
{
    return currentStatus;
}

juce::String R2OneDriveProvider::getDisplayName() const
{
    return "OneDrive";
}

void R2OneDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    auto pathParts = juce::StringArray::fromTokens(filePath, "/", "");
    if (pathParts.isEmpty())
    {
        if (callback) callback(false, "Invalid file path");
        return;
    }

    auto fileName = pathParts[pathParts.size() - 1];
    pathParts.remove(pathParts.size() - 1);

    createFolderPath(pathParts, "root", 0, [this, fileName, data, callback](bool success, juce::String parentId)
    {
        if (success)
        {
            uploadToFolder(fileName, parentId, data, callback);
        }
        else
        {
            if (callback) callback(false, "Failed to create folder path");
        }
    });
}

void R2OneDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(filePath, true);
    
    makeAPIRequest(endpoint, "GET", {}, {}, [this, callback](bool success, int statusCode, const juce::String& responseBody)
    {
        if (!success || statusCode != 200)
        {
            if (callback) callback(false, {}, "File not found or error accessing file: " + responseBody);
            return;
        }

        try
        {
            auto json = juce::JSON::parse(responseBody);
            juce::String downloadUrl = json.getDynamicObject()->getProperty("@microsoft.graph.downloadUrl").toString();
            
            if (downloadUrl.isEmpty())
            {
                if (callback) callback(false, {}, "Could not get download URL");
                return;
            }

            juce::URL url(downloadUrl);
            
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withConnectionTimeoutMs(30000);
            
            juce::Thread::launch([url, options, callback]() {
                if (auto stream = url.createInputStream(options))
                {
                    juce::MemoryBlock downloadedData;
                    stream->readIntoMemoryBlock(downloadedData);
                    juce::MessageManager::callAsync([callback, downloadedData](){ if (callback) callback(true, downloadedData, ""); });
                }
                else
                {
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content"); });
                }
            });
        }
        catch(...)
        {
            juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to parse file metadata"); });
        }
    });
}


void R2OneDriveProvider::setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken)
{
    accessToken = newAccessToken;
    if (newRefreshToken.isNotEmpty())
        refreshToken = newRefreshToken;
    
    tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(3500);
    currentStatus = Status::Authenticated;
    saveTokens();
}

bool R2OneDriveProvider::isTokenValid() const
{
    return accessToken.isNotEmpty() && juce::Time::getCurrentTime() < tokenExpiry;
}

void R2OneDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    if (refreshToken.isEmpty())
    {
        if (callback) callback(false);
        return;
    }

    juce::Thread::launch([this, callback] {
        // === THE FINAL FIX No.1: Public clients MUST NOT send a client_secret. ===
        juce::String postData = "client_id=" + juce::URL::addEscapeChars(clientId, false)
                              + "&scope=" + juce::URL::addEscapeChars("files.readwrite offline_access", false)
                              + "&refresh_token=" + juce::URL::addEscapeChars(refreshToken, false)
                              + "&grant_type=refresh_token";

        // === THE FINAL FIX No.2: Use the /consumers endpoint as requested by the server. ===
        juce::URL tokenUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token");
        tokenUrl = tokenUrl.withPOSTData(postData);
        
        juce::StringPairArray headers;
        headers.set("Content-Type", "application/x-www-form-urlencoded");
        
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
            headerString << headers.getAllKeys()[i] << ": " << headers.getAllValues()[i] << "\r\n";

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                       .withExtraHeaders(headerString)
                       .withConnectionTimeoutMs(30000);

        if (auto stream = tokenUrl.createInputStream(options))
        {
            auto response = stream->readEntireStreamAsString();
            DBG("OneDrive Refresh Token Response: " + response);
            
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject())
            {
                if (obj->hasProperty("access_token"))
                {
                    juce::MessageManager::callAsync([this, obj, callback]() {
                        setTokens(obj->getProperty("access_token").toString(), obj->getProperty("refresh_token").toString());
                        if (callback) callback(true);
                    });
                    return;
                }
            }
        }
        
        juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
    });
}

void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method,
                                        const juce::StringPairArray& headers, const juce::String& body,
                                        std::function<void(bool, int, juce::String)> callback)
{
    if (!isTokenValid())
    {
        refreshAccessToken([=](bool success) {
            if (success)
            {
                makeAPIRequest(endpoint, method, headers, body, callback);
            }
            else
            {
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 401, "Authentication required"); });
            }
        });
        return;
    }

    juce::Thread::launch([=]() {
        juce::URL url(endpoint);
        
        if (method != "GET" && body.isNotEmpty())
            url = url.withPOSTData(body);
        
        juce::StringPairArray requestHeaders = headers;
        requestHeaders.set("Authorization", "Bearer " + accessToken);

        juce::String headerString;
        for (int i = 0; i < requestHeaders.size(); ++i)
            headerString << requestHeaders.getAllKeys()[i] << ": " << requestHeaders.getAllValues()[i] << "\r\n";

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                        .withExtraHeaders(headerString)
                        .withConnectionTimeoutMs(30000)
                        .withHttpRequestCmd(method);
        
        if (auto stream = url.createInputStream(options))
        {
            int statusCode = 0;
            if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get()))
                statusCode = webStream->getStatusCode();
            
            auto responseBody = stream->readEntireStreamAsString();
            
            juce::MessageManager::callAsync([=](){
                if (statusCode >= 200 && statusCode < 300)
                {
                    if (callback) callback(true, statusCode, responseBody);
                }
                else
                {
                    if (callback) callback(false, statusCode, responseBody);
                }
            });
        }
        else
        {
            juce::MessageManager::callAsync([=](){ if (callback) callback(false, 0, "Failed to create request"); });
        }
    });
}

void R2OneDriveProvider::createFolderPath(const juce::StringArray& folderPath, const juce::String& parentId, int pathIndex, std::function<void(bool, juce::String)> callback)
{
    if (pathIndex >= folderPath.size())
    {
        if (callback) callback(true, parentId);
        return;
    }

    auto folderName = folderPath[pathIndex];
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + "/children";
    endpoint += "?$filter=name eq '" + juce::URL::addEscapeChars(folderName, true) + "'";

    makeAPIRequest(endpoint, "GET", {}, {}, [=](bool success, int, const juce::String& response)
    {
        if (!success) { if (callback) callback(false, "Failed to check folder"); return; }
        
        auto json = juce::JSON::parse(response);
        auto value = json.getDynamicObject()->getProperty("value");
        if (value.isArray() && value.getArray()->size() > 0)
        {
            juce::String existingId = value[0].getDynamicObject()->getProperty("id").toString();
            createFolderPath(folderPath, existingId, pathIndex + 1, callback);
        }
        else
        {
            juce::String createEndpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + "/children";
            
            auto* folderObj = new juce::DynamicObject();
            folderObj->setProperty("name", folderName);
            folderObj->setProperty("folder", new juce::DynamicObject());
            folderObj->setProperty("@microsoft.graph.conflictBehavior", "fail");

            juce::StringPairArray headers;
            headers.set("Content-Type", "application/json");

            makeAPIRequest(createEndpoint, "POST", headers, juce::JSON::toString(juce::var(folderObj)), [=](bool createSuccess, int, const juce::String& createResponse)
            {
                if (!createSuccess) { if (callback) callback(false, "Failed to create folder"); return; }
                
                auto newJson = juce::JSON::parse(createResponse);
                juce::String newId = newJson.getDynamicObject()->getProperty("id").toString();
                createFolderPath(folderPath, newId, pathIndex + 1, callback);
            });
        }
    });
}

void R2OneDriveProvider::uploadToFolder(const juce::String& fileName, const juce::String& parentId, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + ":/" + juce::URL::addEscapeChars(fileName, true) + ":/content";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/octet-stream");
    
    juce::String body = juce::String::createStringFromData(data.getData(), (int)data.getSize());

    makeAPIRequest(endpoint, "PUT", headers, body, [callback](bool success, int, const juce::String& response)
    {
        if (callback) callback(success, success ? "Upload successful" : "Upload failed: " + response);
    });
}

void R2OneDriveProvider::listFiles(const juce::String&, FileListCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "Not implemented"); }); }
void R2OneDriveProvider::uploadFile(const juce::String&, const juce::MemoryBlock&, const juce::String&, FileOperationCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, "Not implemented: use uploadFileByPath"); }); }
void R2OneDriveProvider::downloadFile(const juce::String&, DownloadCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "Not implemented: use downloadFileByPath"); }); }
void R2OneDriveProvider::deleteFile(const juce::String&, FileOperationCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, "Not implemented"); }); }
void R2OneDriveProvider::createFolder(const juce::String&, const juce::String&, FileOperationCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, "Not implemented"); }); }

void R2OneDriveProvider::saveTokens()
{
    auto tokenFile = getTokenFile();
    auto* tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", tokenExpiry.toMilliseconds());
    tokenFile.replaceWithText(juce::JSON::toString(juce::var(tokenData)));
}

bool R2OneDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
    if (!tokenFile.exists()) return false;
    
    auto parsed = juce::JSON::parse(tokenFile.loadFileAsString());
    if(auto* obj = parsed.getDynamicObject())
    {
        accessToken = obj->getProperty("access_token").toString();
        refreshToken = obj->getProperty("refresh_token").toString();
        tokenExpiry = juce::Time(static_cast<juce::int64>(obj->getProperty("token_expiry")));
        
        // This is the correct logic: if we have a refresh token, we are considered "logged in"
        // even if the access token is expired. The silent refresh will handle it.
        if (refreshToken.isNotEmpty())
        {
            currentStatus = Status::Authenticated;
            return true;
        }
    }
    return false;
}

juce::File R2OneDriveProvider::getTokenFile() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto cloudDocDir = appDataDir.getChildFile("CloudDoc");
    if (!cloudDocDir.exists()) cloudDocDir.createDirectory();
    return cloudDocDir.getChildFile("onedrive_tokens.json");
}

} // namespace r2juce
