#include "R2OneDriveProvider.h"
#include "R2LocalStorageProvider.h" // Needed for static_cast

namespace r2juce {

juce::String R2OneDriveProvider::buildEscapedPath(const juce::String& rawPath)
{
    juce::StringArray pathParts = juce::StringArray::fromTokens(rawPath, "/", "");
    pathParts.removeEmptyStrings();
    
    juce::String escapedPath;
    for (const auto& part : pathParts)
    {
        if (escapedPath.isNotEmpty())
            escapedPath += "/";
        escapedPath += juce::URL::addEscapeChars(part, false);
    }
    return escapedPath;
}

R2OneDriveProvider::R2OneDriveProvider(const juce::String& cid, const juce::String& csecret)
    : clientId(cid), clientSecret(csecret)
{
    currentStatus = Status::NotAuthenticated;
    loadTokens();
}

R2OneDriveProvider::~R2OneDriveProvider()
{
    if (isUploading)
    {
        juce::ScopedLock lock(uploadLock);
        isUploading = false;
    }
}

R2CloudStorageProvider::ServiceType R2OneDriveProvider::getServiceType() const
{
    return ServiceType::OneDrive;
}

void R2OneDriveProvider::authenticate(AuthCallback callback)
{
    if (isTokenValid()) {
        currentStatus = Status::Authenticated;
        if (callback) juce::MessageManager::callAsync([callback](){ callback(true, "Already authenticated"); });
        return;
    }
    
    if (refreshToken.isNotEmpty())
    {
        currentStatus = Status::Authenticating;
        auto self = shared_from_this();

        refreshAccessToken([this, self, callback](bool success, juce::String errorMessage)
        {
            juce::MessageManager::callAsync([this, self, success, errorMessage, callback]() {
                currentStatus = success ? Status::Authenticated : Status::NotAuthenticated;
                if (callback) callback(success, success ? "Refreshed token successfully" : errorMessage);
            });
        });
        return;
    }
    
    currentStatus = Status::NotAuthenticated;
    if (callback) juce::MessageManager::callAsync([callback](){ callback(false, "device flow authentication required"); });
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

R2CloudStorageProvider::Status R2OneDriveProvider::getAuthStatus() const {
    return currentStatus;
}
juce::String R2OneDriveProvider::getDisplayName() const {
    return "OneDrive";
}
bool R2OneDriveProvider::isTokenValid() const {
    return accessToken.isNotEmpty() && juce::Time::getCurrentTime() < tokenExpiry;
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

void R2OneDriveProvider::configureCaching(std::shared_ptr<R2LocalStorageProvider> newCacheProvider)
{
    DBG("R2OneDriveProvider::configureCaching called. newCacheProvider is " + juce::String(newCacheProvider ? "valid" : "null"));
    cacheProvider = newCacheProvider;
}

bool R2OneDriveProvider::isCachingEnabled() const
{
    bool enabled = cacheProvider != nullptr;
    DBG("R2OneDriveProvider::isCachingEnabled check. Result: " + juce::String(enabled ? "true" : "false"));
    return enabled;
}

void R2OneDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    if (isCachingEnabled())
    {
        cacheProvider->uploadFileByPath(filePath, data, [this, filePath, callback](bool success, const juce::String& errorMessage) {
            if (success)
            {
                triggerBackgroundUpload(filePath);
            }
            if (callback)
            {
                callback(success, errorMessage);
            }
        });
    }
    else
    {
        uploadDirectToCloud(filePath, data, callback);
    }
}

bool R2OneDriveProvider::uploadFileByPathSync(const juce::String& filePath, const juce::MemoryBlock& data)
{
    if (filePath.isEmpty()) return false;
    if (!isTokenValid()) return false;

    juce::String escapedPath = buildEscapedPath(filePath);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + escapedPath + ":/content";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/octet-stream");
    headers.set("Authorization", "Bearer " + accessToken);

    juce::URL url(endpoint);
    url = url.withPOSTData(data);
    
    juce::String headerString;
    for (int i = 0; i < headers.size(); ++i)
        headerString << headers.getAllKeys()[i] << ": " << headers.getAllValues()[i] << "\r\n";

    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                   .withExtraHeaders(headerString)
                   .withConnectionTimeoutMs(15000)
                   .withHttpRequestCmd("PUT");

    if (auto stream = url.createInputStream(options))
    {
        int statusCode = 0;
        if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get()))
        {
            statusCode = webStream->getStatusCode();
        }
        return (statusCode >= 200 && statusCode < 300);
    }
    
    return false;
}

void R2OneDriveProvider::uploadDirectToCloud(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    auto startTime = juce::Time::getMillisecondCounterHiRes();
    if (filePath.isEmpty())
    {
        if (callback) juce::MessageManager::callAsync([callback](){ callback(false, "File path cannot be empty."); });
        return;
    }

    juce::String escapedPath = buildEscapedPath(filePath);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + escapedPath + ":/content";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/octet-stream");

    makeAPIRequest(endpoint, "PUT", headers, data, [callback, filePath, startTime](bool success, int statusCode, const juce::var& response) {
        auto endTime = juce::Time::getMillisecondCounterHiRes();
        DBG("OneDrive: uploadDirectToCloud for " + filePath + " took " + juce::String(endTime - startTime) + " ms.");
        if (success)
        {
            if (callback) juce::MessageManager::callAsync([callback](){ callback(true, "Upload successful"); });
        }
        else
        {
            juce::String errorMessage = "Upload failed for file: " + filePath + ". Status: " + juce::String(statusCode) + ", Response: " + juce::JSON::toString(response);
            if (callback) juce::MessageManager::callAsync([callback, errorMessage](){ callback(false, errorMessage); });
        }
    });
}

void R2OneDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    auto startTime = juce::Time::getMillisecondCounterHiRes();
    if (isCachingEnabled())
    {
        auto deliveredInitialContent = std::make_shared<std::atomic<bool>>(false);
        
        cacheProvider->downloadFileByPath(filePath, [this, filePath, callback, deliveredInitialContent, startTime](bool success, const juce::MemoryBlock& data, const juce::String&) {
            if (success)
            {
                DBG("Cache hit for: " + filePath + ". Delivering cached content immediately.");
                deliveredInitialContent->store(true);
                if (callback) callback(true, data, "");
            }
            else
            {
                DBG("Cache miss for: " + filePath);
            }

            DBG("Checking cloud for metadata for: " + filePath);
            getCloudFileMetadata(filePath, [this, filePath, callback, deliveredInitialContent, startTime](bool metaSuccess, const FileInfo& cloudInfo) {
                if (metaSuccess)
                {
                    DBG("Successfully fetched cloud metadata for: " + filePath);
                    if (auto* localCacheProvider = dynamic_cast<R2LocalStorageProvider*>(cacheProvider.get()))
                    {
                        juce::File cacheFile = localCacheProvider->getLocalStorageRoot().getChildFile(filePath);

                        if (!cacheFile.exists() || cloudInfo.modifiedTime > cacheFile.getLastModificationTime())
                        {
                            DBG("Cloud file is newer or cache does not exist. Downloading from cloud...");
                            juce::String downloadUrl = cloudInfo.id;
                            juce::URL url(downloadUrl);
                            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(30000);
                            
                            juce::Thread::launch([this, url, options, callback, filePath, deliveredInitialContent, startTime]() {
                                std::unique_ptr<juce::InputStream> stream (url.createInputStream(options));
                                if (stream != nullptr) {
                                    juce::MemoryBlock downloadedData;
                                    stream->readIntoMemoryBlock(downloadedData);
                                    DBG("Successfully downloaded newer version of " + filePath + ". Updating cache and delivering content.");
                                    cacheProvider->uploadFileByPath(filePath, downloadedData, nullptr);
                                    auto endTime = juce::Time::getMillisecondCounterHiRes();
                                    DBG("OneDrive: downloadFileByPath (cache miss, cloud download) for " + filePath + " took " + juce::String(endTime - startTime) + " ms.");
                                    juce::MessageManager::callAsync([callback, downloadedData](){ if (callback) callback(true, downloadedData, ""); });
                                } else if (!deliveredInitialContent->load()) {
                                    DBG("Failed to download " + filePath + " and no cached version was delivered.");
                                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content."); });
                                }
                            });
                        }
                        else
                        {
                             DBG("Cache is up-to-date for: " + filePath);
                        }
                    }
                }
                else if (!deliveredInitialContent->load())
                {
                    DBG("Failed to get cloud metadata for " + filePath + " and no cached version was delivered.");
                    if (callback) callback(false, {}, "File not in cache and could not check cloud.");
                }
            });
        });
    }
    else
    {
        if (filePath.isEmpty())
        {
            if (callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "File path cannot be empty."); });
            return;
        }
        getCloudFileMetadata(filePath, [this, callback, startTime, filePath](bool success, const FileInfo& fileInfo) {
            if (!success) {
                if (callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "File not found or metadata request failed."); });
                return;
            }
            
            juce::URL url(fileInfo.id);
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(30000);
            
            juce::Thread::launch([url, options, callback, startTime, filePath]() {
                std::unique_ptr<juce::InputStream> stream (url.createInputStream(options));
                if (stream != nullptr) {
                    juce::MemoryBlock downloadedData;
                    stream->readIntoMemoryBlock(downloadedData);
                    auto endTime = juce::Time::getMillisecondCounterHiRes();
                    DBG("OneDrive: downloadFileByPath (no cache) for " + filePath + " took " + juce::String(endTime - startTime) + " ms.");
                    juce::MessageManager::callAsync([callback, downloadedData](){ if (callback) callback(true, downloadedData, ""); });
                } else {
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content."); });
                }
            });
        });
    }
}

void R2OneDriveProvider::triggerBackgroundUpload(const juce::String& filePath)
{
    juce::ScopedLock lock(uploadLock);
    pendingUploadPath = filePath;

    if (!isUploading)
    {
        isUploading = true;
        juce::Thread::launch([this] { performUpload(); });
    }
}

void R2OneDriveProvider::performUpload()
{
    while (true)
    {
        juce::String path_to_upload;
        {
            juce::ScopedLock lock(uploadLock);
            if (!isUploading) break;
            path_to_upload = pendingUploadPath;
            pendingUploadPath.clear();
        }

        if (onSyncStatusChanged)
            onSyncStatusChanged(R2CloudStorageProvider::SyncStatus::Syncing, "Uploading " + path_to_upload);
        
        juce::MemoryBlock dataToUpload;
        cacheProvider->downloadFileByPath(path_to_upload, [&dataToUpload](bool success, const juce::MemoryBlock& data, const juce::String&) {
            if (success) dataToUpload = data;
        });

        if (dataToUpload.getSize() > 0)
        {
             uploadDirectToCloud(path_to_upload, dataToUpload, [this, path_to_upload](bool success, const juce::String& errorMessage) {
                if (onSyncStatusChanged)
                {
                    if (success)
                        onSyncStatusChanged(R2CloudStorageProvider::SyncStatus::Synced, "Synced: " + path_to_upload);
                    else
                        onSyncStatusChanged(R2CloudStorageProvider::SyncStatus::SyncError, "Sync Error: " + errorMessage);
                }
            });
        }
        else
        {
             if (onSyncStatusChanged)
                onSyncStatusChanged(R2CloudStorageProvider::SyncStatus::SyncError, "Sync Error: Could not read from cache for upload.");
        }

        {
            juce::ScopedLock lock(uploadLock);
            if (pendingUploadPath.isEmpty())
            {
                isUploading = false;
                if (onSyncStatusChanged) onSyncStatusChanged(R2CloudStorageProvider::SyncStatus::Idle, "");
                return;
            }
        }
    }
}


void R2OneDriveProvider::getCloudFileMetadata(const juce::String& filePath, std::function<void(bool, const FileInfo&)> callback)
{
    juce::String escapedPath = R2OneDriveProvider::buildEscapedPath(filePath);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + escapedPath;
    
    makeAPIRequest(endpoint, "GET", {}, juce::MemoryBlock(), [callback](bool success, int, const juce::var& responseVar)
    {
        if (success && responseVar.isObject()) {
            if (auto* obj = responseVar.getDynamicObject()) {
                FileInfo info;
                info.name = obj->getProperty("name").toString();
                info.id = obj->getProperty("@microsoft.graph.downloadUrl").toString();
                info.modifiedTime = juce::Time::fromISO8601(obj->getProperty("lastModifiedDateTime").toString());
                info.size = (juce::int64) obj->getProperty("size");
                info.isFolder = obj->hasProperty("folder");
                if (callback) callback(true, info);
                return;
            }
        }
        if (callback) callback(false, {});
    });
}

void R2OneDriveProvider::refreshAccessToken(AuthCallback callback)
{
    if (refreshToken.isEmpty()) {
        if (callback) juce::MessageManager::callAsync([callback](){ callback(false, "No refresh token available."); });
        return;
    }

    auto selfWeak = std::weak_ptr<R2CloudStorageProvider>(shared_from_this());
    
    juce::String clientIdCopy = clientId;
    juce::String refreshTokenCopy = refreshToken;

    juce::Thread::launch([selfWeak, callback, clientIdCopy, refreshTokenCopy] {
        if (auto selfShared = selfWeak.lock()) {
            juce::String postDataString = "client_id=";
            postDataString += juce::URL::addEscapeChars(clientIdCopy, true);
            postDataString += "&scope=";
            postDataString += juce::URL::addEscapeChars("files.ReadWrite offline_access", true);
            postDataString += "&refresh_token=";
            postDataString += juce::URL::addEscapeChars(refreshTokenCopy, true);
            postDataString += "&grant_type=refresh_token";

            juce::URL tokenUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token");
            tokenUrl = tokenUrl.withPOSTData(postDataString);

            juce::StringPairArray headers; headers.set("Content-Type", "application/x-www-form-urlencoded");
            juce::String headerString;
            for (int i = 0; i < headers.size(); ++i) headerString << headers.getAllKeys()[i] << ": " << headers.getAllValues()[i] << "\r\n";

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                           .withExtraHeaders(headerString)
                           .withConnectionTimeoutMs(30000)
                           .withHttpRequestCmd("POST");

            std::unique_ptr<juce::InputStream> stream (tokenUrl.createInputStream(options));
            if (stream != nullptr)
            {
                juce::String responseString = stream->readEntireStreamAsString();
                int statusCode = 0;
                if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get()))
                    statusCode = webStream->getStatusCode();

                juce::var jsonResponse = juce::JSON::parse(responseString);
                
                if (statusCode >= 200 && statusCode < 300 && jsonResponse.isObject())
                {
                    if (jsonResponse.getDynamicObject()->hasProperty("access_token")) {
                        juce::MessageManager::callAsync([selfWeak, jsonResponse, callback]() {
                            if (auto providerPtr = dynamic_cast<R2OneDriveProvider*>(selfWeak.lock().get())) {
                                if (auto* validObj = jsonResponse.getDynamicObject())
                                {
                                    providerPtr->setTokens(
                                        validObj->getProperty("access_token").toString(),
                                        validObj->getProperty("refresh_token").toString()
                                    );
                                    if (callback) callback(true, "");
                                }
                            }
                        });
                        return;
                    }
                }
                
                juce::String errorMessage = "Refresh token failed. Status: " + juce::String(statusCode) + ", Response: " + responseString;
                juce::MessageManager::callAsync([callback, errorMessage](){ if (callback) callback(false, errorMessage); });
            } else {
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false, "Network error: Stream creation failed."); });
            }
        }
    });
}

void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method, const juce::StringPairArray& headers, const juce::MemoryBlock& body, std::function<void(bool, int, const juce::var&)> callback)
{
    if (!isTokenValid())
    {
        auto self = shared_from_this();
        refreshAccessToken([this, self, endpoint, method, headers, body, callback](bool success, juce::String refreshErrorMessage) {
            if (success) this->makeAPIRequest(endpoint, method, headers, body, callback);
            else juce::MessageManager::callAsync([callback, refreshErrorMessage]() { if (callback) callback(false, 401, juce::var(refreshErrorMessage)); });
        });
        return;
    }

    auto selfWeak = std::weak_ptr<R2CloudStorageProvider>(shared_from_this());
    
    juce::String accessTokenCopy = accessToken;

    juce::Thread::launch([selfWeak, callback, accessTokenCopy, endpoint, method, body, headers] () mutable {
        if (auto selfShared = selfWeak.lock()) {
            juce::URL url(endpoint);
            
            bool isBodyRequest = (method == "PUT" || method == "POST" || method == "PATCH");
            if (isBodyRequest && body.getSize() > 0)
            {
                url = url.withPOSTData(body);
            }

            juce::StringPairArray requestHeaders = headers;
            requestHeaders.set("Authorization", "Bearer " + accessTokenCopy);

            juce::String headerString;
            for (int i = 0; i < requestHeaders.size(); ++i) headerString << requestHeaders.getAllKeys()[i] << ": " << requestHeaders.getAllValues()[i] << "\r\n";

            auto paramHandling = isBodyRequest ? juce::URL::ParameterHandling::inPostData
                                               : juce::URL::ParameterHandling::inAddress;

            auto options = juce::URL::InputStreamOptions(paramHandling)
                           .withExtraHeaders(headerString)
                           .withConnectionTimeoutMs(30000)
                           .withHttpRequestCmd(method);
                           
            std::unique_ptr<juce::InputStream> stream (url.createInputStream(options));
            if (stream != nullptr)
            {
                juce::String responseString = stream->readEntireStreamAsString();
                
                int statusCode = 0;
                if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get()))
                    statusCode = webStream->getStatusCode();

                juce::var responseJson = responseString.isNotEmpty() ? juce::JSON::parse(responseString) : juce::var();

                juce::MessageManager::callAsync([selfWeak, callback, statusCode, responseJson]() {
                    if (selfWeak.lock())
                    {
                        if (callback) callback(statusCode >= 200 && statusCode < 300, statusCode, responseJson);
                    }
                });
            } else {
                juce::MessageManager::callAsync([selfWeak, callback]() {
                    if (selfWeak.lock())
                    {
                        if (callback) callback(false, 0, juce::var("Network error: Stream creation failed."));
                    }
                });
            }
        }
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
    if (!tokenFile.getParentDirectory().exists())
        tokenFile.getParentDirectory().createDirectory();

    auto* tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", tokenExpiry.toMilliseconds());
    tokenFile.replaceWithText(juce::JSON::toString(juce::var(tokenData), true));
}

bool R2OneDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
    DBG(tokenFile.getFullPathName());
    if (!tokenFile.existsAsFile()) return false;
    
    juce::var parsed = juce::JSON::parse(tokenFile);
    if(auto* obj = parsed.getDynamicObject()) {
        accessToken = obj->getProperty("access_token").toString();
        refreshToken = obj->getProperty("refresh_token").toString();
        tokenExpiry = juce::Time(static_cast<juce::int64>(obj->getProperty("token_expiry")));
        
        if (refreshToken.isNotEmpty()) {
            currentStatus = Status::Authenticated;
            return true;
        }
    }
    return false;
}

juce::File R2OneDriveProvider::getTokenFile() const
{
#if JUCE_MAC || JUCE_IOS
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("CloudDoc")
#else
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("STUDIO-R/CloudDoc")
#endif
        .getChildFile("onedrive_tokens.json");
}

} // namespace r2juce

