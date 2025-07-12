#include "R2OneDriveProvider.h"

namespace r2juce {

namespace
{
    juce::String buildEscapedPath(const juce::String& rawPath)
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
}

R2OneDriveProvider::R2OneDriveProvider(const juce::String& cid, const juce::String& csecret)
    : clientId(cid), clientSecret(csecret)
{
    currentStatus = Status::NotAuthenticated;
    loadTokens();
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
            juce::String postDataString = "client_id=" + juce::URL::addEscapeChars(clientIdCopy, true)
                                        + "&scope=" + juce::URL::addEscapeChars("files.ReadWrite offline_access", true)
                                        + "&refresh_token=" + juce::URL::addEscapeChars(refreshTokenCopy, true)
                                        + "&grant_type=refresh_token";

            juce::URL tokenUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token");
            tokenUrl = tokenUrl.withPOSTData(postDataString);

            juce::StringPairArray headers; headers.set("Content-Type", "application/x-www-form-urlencoded");
            juce::String headerString;
            for (int i = 0; i < headers.size(); ++i) headerString << headers.getAllKeys()[i] << ": " << headers.getAllValues()[i] << "\r\n";

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inPostData)
                           .withExtraHeaders(headerString)
                           .withConnectionTimeoutMs(30000)
                           .withHttpRequestCmd("POST");

            if (auto stream = tokenUrl.createInputStream(options))
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
                           
            if (auto stream = url.createInputStream(options))
            {
                int statusCode = 0;
                if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get()))
                    statusCode = webStream->getStatusCode();

                juce::String responseString = stream->readEntireStreamAsString();
                juce::var responseJson = responseString.isNotEmpty() ? juce::JSON::parse(responseString) : juce::var();

                juce::MessageManager::callAsync([callback, statusCode, responseJson]() {
                    if (callback) callback(statusCode >= 200 && statusCode < 300, statusCode, responseJson);
                });
            } else {
                juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 0, juce::var("Network error: Stream creation failed.")); });
            }
        }
    });
}

void R2OneDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    if (filePath.isEmpty())
    {
        if (callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "File path cannot be empty."); });
        return;
    }
    
    juce::String escapedPath = buildEscapedPath(filePath);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + escapedPath;
    
    makeAPIRequest(endpoint, "GET", {}, juce::MemoryBlock(), [this, callback, filePath](bool success, int statusCode, const juce::var& responseVar)
    {
        if (!success || statusCode != 200) {
            if (callback) juce::MessageManager::callAsync([callback, statusCode](){ callback(false, {}, "File not found or metadata request failed. Status: " + juce::String(statusCode)); });
            return;
        }
        if (auto* obj = responseVar.getDynamicObject()) {
            juce::String downloadUrl = obj->getProperty("@microsoft.graph.downloadUrl").toString();
            if (downloadUrl.isEmpty()) {
                if (callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "Could not get download URL"); });
                return;
            }
            
            juce::URL url(downloadUrl);
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(30000);
            
            juce::Thread::launch([url, options, callback]() {
                if (auto stream = url.createInputStream(options)) {
                    juce::MemoryBlock downloadedData;
                    stream->readIntoMemoryBlock(downloadedData);
                    juce::MessageManager::callAsync([callback, downloadedData](){ if (callback) callback(true, downloadedData, ""); });
                } else {
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content."); });
                }
            });
        } else {
            if (callback) juce::MessageManager::callAsync([callback](){ callback(false, {}, "Failed to parse file metadata."); });
        }
    });
}

void R2OneDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    if (filePath.isEmpty())
    {
        if (callback) juce::MessageManager::callAsync([callback](){ callback(false, "File path cannot be empty."); });
        return;
    }

    juce::String escapedPath = buildEscapedPath(filePath);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + escapedPath + ":/content";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/octet-stream");

    makeAPIRequest(endpoint, "PUT", headers, data, [callback, filePath](bool success, int statusCode, const juce::var& response) {
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
    tokenFile.replaceWithText(juce::JSON::toString(juce::var(tokenData), true));
}

bool R2OneDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
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
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto cloudDocDir = appDataDir.getChildFile("R2JuceCloudApp");
    if (!cloudDocDir.exists()) cloudDocDir.createDirectory();
    return cloudDocDir.getChildFile("onedrive_tokens.json");
}

} // namespace r2juce

