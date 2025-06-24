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
    if (isTokenValid()) {
        currentStatus = Status::Authenticated;
        if (callback) callback(true, "Already authenticated");
        return;
    }
    
    if (refreshToken.isNotEmpty())
    {
        currentStatus = Status::Authenticating;
        auto self = shared_from_this();
        refreshAccessToken([this, self, callback](bool success)
        {
            this->currentStatus = success ? Status::Authenticated : Status::NotAuthenticated;
            juce::MessageManager::callAsync([callback, success](){
                if (callback) callback(success, success ? "Refreshed token" : "Refresh failed");
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

R2CloudStorageProvider::Status R2OneDriveProvider::getAuthStatus() const { return currentStatus; }
juce::String R2OneDriveProvider::getDisplayName() const { return "OneDrive"; }
bool R2OneDriveProvider::isTokenValid() const { return accessToken.isNotEmpty() && juce::Time::getCurrentTime() < tokenExpiry; }

void R2OneDriveProvider::setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken)
{
    accessToken = newAccessToken;
    if (newRefreshToken.isNotEmpty())
        refreshToken = newRefreshToken;
    
    tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(3500);
    currentStatus = Status::Authenticated;
    saveTokens();
}

void R2OneDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    if (refreshToken.isEmpty()) { if (callback) callback(false); return; }

    auto self = shared_from_this();
    juce::Thread::launch([this, self, callback] {
        juce::String postData = "client_id=" + this->clientId
                              + "&scope=" + juce::URL::addEscapeChars("files.readwrite offline_access", false)
                              + "&refresh_token=" + this->refreshToken
                              + "&grant_type=refresh_token";

        juce::URL tokenUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token");
        tokenUrl = tokenUrl.withPOSTData(postData);
        
        juce::StringPairArray headers; headers.set("Content-Type", "application/x-www-form-urlencoded");
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i) headerString << headers.getAllKeys()[i] << ": " << headers.getAllValues()[i] << "\r\n";

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData).withExtraHeaders(headerString).withConnectionTimeoutMs(30000);

        if (auto stream = tokenUrl.createInputStream(options)) {
            auto response = stream->readEntireStreamAsString();
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject()) {
                if (obj->hasProperty("access_token")) {
                    juce::MessageManager::callAsync([this, self, obj, callback]() {
                        this->setTokens(obj->getProperty("access_token").toString(), obj->getProperty("refresh_token").toString());
                        if (callback) callback(true);
                    });
                    return;
                }
            }
        }
        juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
    });
}

void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method, const juce::StringPairArray& headers, const juce::String& body, std::function<void(bool, int, const juce::var&)> callback)
{
    if (!isTokenValid())
    {
        auto self = shared_from_this();
        refreshAccessToken([this, self, endpoint, method, headers, body, callback](bool success) {
            if (success) this->makeAPIRequest(endpoint, method, headers, body, callback);
            else juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 401, juce::var()); });
        });
        return;
    }

    auto self = shared_from_this();
    juce::Thread::launch([this, self, endpoint, method, headers, body, callback]()
    {
        juce::URL url(endpoint);
        // Changed: For string body, use withPOSTData. For MemoryBlock, use a different overload.
        if (method != "GET" && body.isNotEmpty()) url = url.withPOSTData(body);
        
        juce::StringPairArray requestHeaders = headers;
        requestHeaders.set("Authorization", "Bearer " + this->accessToken);

        juce::String headerString;
        for (int i = 0; i < requestHeaders.size(); ++i) headerString << requestHeaders.getAllKeys()[i] << ": " << requestHeaders.getAllValues()[i] << "\r\n";

        // JUCE8対応: InputStreamOptionsからwithParameterHandlingを削除
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withExtraHeaders(headerString).withConnectionTimeoutMs(30000).withHttpRequestCmd(method);
        
        if (auto stream = url.createInputStream(options)) {
            int statusCode = 0;
            if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get())) statusCode = webStream->getStatusCode();
            auto responseJson = juce::JSON::parse(*stream);
            juce::MessageManager::callAsync([callback, statusCode, responseJson]() {
                if (statusCode >= 200 && statusCode < 300) { if (callback) callback(true, statusCode, responseJson); }
                else { if (callback) callback(false, statusCode, responseJson); }
            });
        } else {
            juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 0, juce::var()); });
        }
    });
}

// New overload for makeAPIRequest that accepts juce::MemoryBlock for the body
// New overload for makeAPIRequest that accepts juce::MemoryBlock for the body
void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method, const juce::StringPairArray& headers, const juce::MemoryBlock& body, std::function<void(bool, int, const juce::var&)> callback)
{
    if (!isTokenValid())
    {
        auto self = shared_from_this();
        refreshAccessToken([this, self, endpoint, method, headers, body, callback](bool success) {
            if (success) this->makeAPIRequest(endpoint, method, headers, body, callback);
            else juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 401, juce::var()); });
        });
        return;
    }

    auto self = shared_from_this();
    juce::Thread::launch([this, self, endpoint, method, headers, body, callback]()
    {
        juce::URL url(endpoint);
        
        // FIX: If the method is PUT/POST/PATCH and body is not empty, use withPOSTData
        // JUCEのwithPOSTDataは、MemoryBlockを受け取るとRAW POSTデータとして送信します。
        if ((method == "PUT" || method == "POST" || method == "PATCH") && body.getSize() > 0)
        {
            url = url.withPOSTData(body);
        }
        
        juce::StringPairArray requestHeaders = headers;
        requestHeaders.set("Authorization", "Bearer " + this->accessToken);

        juce::String headerString;
        for (int i = 0; i < requestHeaders.size(); ++i) headerString << requestHeaders.getAllKeys()[i] << ": " << requestHeaders.getAllValues()[i] << "\r\n";

        // JUCE8対応: InputStreamOptionsからwithParameterHandlingを削除
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withExtraHeaders(headerString)
                       .withConnectionTimeoutMs(30000)
                       .withHttpRequestCmd(method);
                       
        if (auto stream = url.createInputStream(options)) {
            int statusCode = 0;
            if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get())) statusCode = webStream->getStatusCode();
            auto responseJson = juce::JSON::parse(*stream);
            juce::MessageManager::callAsync([callback, statusCode, responseJson]() {
                if (statusCode >= 200 && statusCode < 300) { if (callback) callback(true, statusCode, responseJson); }
                else { if (callback) callback(false, statusCode, responseJson); }
            });
        } else {
            juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 0, juce::var()); });
        }
    });
}

void R2OneDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(filePath, true);
    
    auto self = shared_from_this();
    makeAPIRequest(endpoint, "GET", {}, juce::String(), [this, self, callback](bool success, int statusCode, const juce::var& responseVar) // Empty string for body
    {
        if (!success || statusCode != 200) { if (callback) callback(false, {}, "File not found"); return; }
        if (auto* obj = responseVar.getDynamicObject()) {
            juce::String downloadUrl = obj->getProperty("@microsoft.graph.downloadUrl").toString();
            if (downloadUrl.isEmpty()) { if (callback) callback(false, {}, "Could not get download URL"); return; }
            
            juce::URL url(downloadUrl);
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(30000);
            
            juce::Thread::launch([url, options, callback]() {
                if (auto stream = url.createInputStream(options)) {
                    juce::MemoryBlock downloadedData;
                    stream->readIntoMemoryBlock(downloadedData);
                    juce::MessageManager::callAsync([callback, downloadedData](){ if (callback) callback(true, downloadedData, ""); });
                } else {
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content"); });
                }
            });
        } else {
            if (callback) callback(false, {}, "Failed to parse file metadata");
        }
    });
}

void R2OneDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    auto pathParts = juce::StringArray::fromTokens(filePath, "/", "");
    if (pathParts.isEmpty()) { if (callback) callback(false, "Invalid file path"); return; }
    auto fileName = pathParts[pathParts.size() - 1];
    pathParts.remove(pathParts.size() - 1);
    auto self = shared_from_this();
    createFolderPath(pathParts, "root", 0, [this, self, fileName, data, callback](bool success, juce::String parentId) {
        if (success) { this->uploadToFolder(fileName, parentId, data, callback); }
        else { if (callback) callback(false, "Failed to create folder path"); }
    });
}

// === FIX START ===
// This is the full, correct, recursive implementation of createFolderPath.
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

    auto self = shared_from_this();
    makeAPIRequest(endpoint, "GET", {}, juce::String(), [this, self, folderPath, pathIndex, folderName, parentId, callback](bool success, int, const juce::var& responseVar) // Empty string for body
    {
        if (!success) { if (callback) callback(false, "Failed to check folder"); return; }
        
        auto value = responseVar.getDynamicObject()->getProperty("value");
        if (value.isArray() && value.getArray()->size() > 0)
        {
            juce::String existingId = value[0].getDynamicObject()->getProperty("id").toString();
            this->createFolderPath(folderPath, existingId, pathIndex + 1, callback);
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

            this->makeAPIRequest(createEndpoint, "POST", headers, juce::JSON::toString(juce::var(folderObj)), [this, self, folderPath, pathIndex, callback](bool createSuccess, int, const juce::var& createResponseVar)
            {
                if (!createSuccess) { if (callback) callback(false, "Failed to create folder"); return; }
                
                juce::String newId = createResponseVar.getDynamicObject()->getProperty("id").toString();
                this->createFolderPath(folderPath, newId, pathIndex + 1, callback);
            });
        }
    });
}
// === FIX END ===


void R2OneDriveProvider::uploadToFolder(const juce::String& fileName, const juce::String& parentId, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + ":/" + juce::URL::addEscapeChars(fileName, true) + ":/content";
    juce::StringPairArray headers; headers.set("Content-Type", "application/octet-stream");
    
    // FIX: Pass MemoryBlock directly to the correct makeAPIRequest overload
    makeAPIRequest(endpoint, "PUT", headers, data, [callback](bool success, int, const juce::var& response) {
        if (callback) callback(success, success ? "Upload successful" : "Upload failed: " + juce::JSON::toString(response));
    });
}

void R2OneDriveProvider::listFiles(const juce::String&, FileListCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "Not implemented"); }); }
void R2OneDriveProvider::uploadFile(const juce::String&, const juce::MemoryBlock&, const juce::String&, FileOperationCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, "Not implemented"); }); }
void R2OneDriveProvider::downloadFile(const juce::String&, DownloadCallback callback) { if(callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "Not implemented"); }); }
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
    if(auto* obj = parsed.getDynamicObject()) {
        accessToken = obj->getProperty("access_token").toString();
        refreshToken = obj->getProperty("refresh_token").toString();
        tokenExpiry = juce::Time(static_cast<juce::int64>(obj->getProperty("token_expiry")));
        if (refreshToken.isNotEmpty()) { currentStatus = Status::Authenticated; return true; }
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
