#include "R2OneDriveProvider.h"

namespace r2juce {

R2OneDriveProvider::R2OneDriveProvider()
{
    currentStatus = Status::NotAuthenticated;
    loadTokens();
}

void R2OneDriveProvider::setClientCredentials(const juce::String& cid, const juce::String& csecret)
{
    // Removed juce::ScopedLock
    clientId = cid;
    clientSecret = csecret;
}

void R2OneDriveProvider::authenticate(AuthCallback callback)
{
    DBG("R2OneDriveProvider::authenticate called. Current status: " + juce::String(static_cast<int>(currentStatus)));

    // Removed juce::ScopedLock
    if (isTokenValid()) { // isTokenValid will acquire its own lock, it's safe to call here.
        currentStatus = Status::Authenticated;
        DBG("Token is valid, already authenticated.");
        if (callback) callback(true, "Already authenticated");
        return;
    }
    
    if (refreshToken.isNotEmpty())
    {
        currentStatus = Status::Authenticating;
        DBG("Refresh token found, attempting to refresh access token.");
        auto self = shared_from_this(); // shared_ptr ensures R2OneDriveProvider instance lives
        // Removed sl.release();
        refreshAccessToken([this, self, callback](bool success)
        {
            // This callback runs on the message thread after refreshAccessToken completes its work
            juce::MessageManager::callAsync([self, success, callback]() {
                if (auto providerPtr = dynamic_cast<R2OneDriveProvider*>(self.get())) {
                    // Removed juce::ScopedLock
                    providerPtr->currentStatus = success ? Status::Authenticated : Status::NotAuthenticated;
                    DBG("Refresh token callback completed. New status: " + juce::String(static_cast<int>(providerPtr->currentStatus)));
                    if (callback) callback(success, success ? "Refreshed token" : "Refresh failed");
                } else {
                    DBG("R2OneDriveProvider instance invalid during authenticate callback after refresh.");
                }
            });
        });
        return;
    }
    
    currentStatus = Status::NotAuthenticated;
    DBG("No valid access token or refresh token. Device flow authentication required.");
    if (callback) callback(false, "device flow authentication required");
}

void R2OneDriveProvider::signOut()
{
    // Removed juce::ScopedLock
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = juce::Time();
    currentStatus = Status::NotAuthenticated;
    if (auto tokenFile = getTokenFile(); tokenFile.exists())
        tokenFile.deleteFile();
    DBG("Signed out from OneDrive, tokens cleared.");
}

R2CloudStorageProvider::Status R2OneDriveProvider::getAuthStatus() const {
    // Removed juce::ScopedLock
    return currentStatus;
}
juce::String R2OneDriveProvider::getDisplayName() const { return "OneDrive"; }
bool R2OneDriveProvider::isTokenValid() const {
    // Removed juce::ScopedLock
    return accessToken.isNotEmpty() && juce::Time::getCurrentTime() < tokenExpiry;
}

void R2OneDriveProvider::setTokens(const juce::String& newAccessToken, const juce::String& newRefreshToken)
{
    // Removed juce::ScopedLock
    // Reconstruct juce::String from raw UTF8 data to ensure a deep copy and avoid
    // potential issues with juce::String's Copy-on-Write (CoW) optimization in multi-threaded contexts.
    // This guarantees the string content is in new memory.
    accessToken = juce::String::fromUTF8(newAccessToken.toRawUTF8(), (int) newAccessToken.getNumBytesAsUTF8());
    
    if (newRefreshToken.isNotEmpty())
        refreshToken = juce::String::fromUTF8(newRefreshToken.toRawUTF8(), (int) newRefreshToken.getNumBytesAsUTF8());

    tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(3500); // Token expiry set to 3500 seconds (approx 58 minutes)
    currentStatus = Status::Authenticated;
    saveTokens(); // This also needs to be thread-safe for reading/writing tokens (handled by saveTokens logic)
    
    DBG("Tokens set and saved. Access Token length: " + juce::String(accessToken.length()) + ", Refresh Token length: " + juce::String(refreshToken.length()));
}

void R2OneDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    // No lock here at the start, as it might be called from another function already holding a lock,
    // or from a new thread. Individual member variable accesses are locked when copied.
    if (refreshToken.isEmpty()) {
        DBG("Refresh token failed: No refresh token available.");
        if (callback) callback(false);
        return;
    }

    DBG("R2OneDriveProvider::refreshAccessToken: Starting refresh process.");
    auto selfWeak = std::weak_ptr<R2CloudStorageProvider>(shared_from_this());
    
    // Ensure initial member variables are copied safely without CriticalSection.
    // Relying on juce::String's thread-safety for const access, and explicit deep copy for captured vars.
    juce::String clientIdCopy = clientId;
    juce::String refreshTokenCopy = refreshToken;

    juce::Thread::launch([selfWeak, callback, clientIdCopy, refreshTokenCopy] {
        DBG("refreshAccessToken thread started.");
        DBG("refreshAccessToken thread: clientIdCopy len=" + juce::String(clientIdCopy.length()) + ", refreshTokenCopy len=" + juce::String(refreshTokenCopy.length()));

        if (auto selfShared = selfWeak.lock()) { // Lock weak_ptr to ensure object is still valid
            juce::String postDataString = "client_id=" + clientIdCopy
                                        + "&scope=" + juce::URL::addEscapeChars("files.ReadWrite offline_access", false)
                                        + "&refresh_token=" + refreshTokenCopy
                                        + "&grant_type=refresh_token";

            juce::URL tokenUrl("https://login.microsoftonline.com/consumers/oauth2/v2.0/token");
            tokenUrl = tokenUrl.withPOSTData(postDataString);

            juce::StringPairArray headers; headers.set("Content-Type", "application/x-www-form-urlencoded");
            juce::String headerString;
            for (int i = 0; i < headers.size(); ++i) headerString << headers.getAllKeys()[i] << ": " << headers.getAllValues()[i] << "\r\n";

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders(headerString)
                           .withConnectionTimeoutMs(30000)
                           .withHttpRequestCmd("POST");

            DBG("refreshAccessToken: Attempting to create input stream for token request. URL: " + tokenUrl.toString(true));
            std::unique_ptr<juce::InputStream> stream;
            try {
                stream = tokenUrl.createInputStream(options);
            } catch (const std::exception& e) {
                DBG("Exception creating refresh token input stream: " + juce::String(e.what()));
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
                return;
            } catch (...) {
                DBG("Unknown exception creating refresh token input stream.");
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
                return;
            }

            if (stream) {
                DBG("refreshAccessToken: Input stream created, reading response.");
                juce::String responseString = stream->readEntireStreamAsString();
                int statusCode = 0;
                if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get())) {
                    statusCode = webStream->getStatusCode();
                    DBG("refreshAccessToken: HTTP Status Code: " + juce::String(statusCode));
                } else {
                    DBG("refreshAccessToken: Could not cast stream to WebInputStream for status code.");
                }

                DBG("refreshAccessToken: Attempting to parse JSON response. Raw response length: " + juce::String(responseString.length()));
                juce::var jsonResponse = juce::JSON::parse(responseString);
                
                if (statusCode >= 200 && statusCode < 300) {
                    if (jsonResponse.isObject())
                    {
                        if (auto* obj = jsonResponse.getDynamicObject()) {
                            if (obj->hasProperty("access_token")) {
                                DBG("refreshAccessToken: Successfully obtained access token.");
                                juce::MessageManager::callAsync([selfWeak, obj, callback]() {
                                    if (auto providerPtr = dynamic_cast<R2OneDriveProvider*>(selfWeak.lock().get())) {
                                        providerPtr->setTokens(
                                            juce::String::fromUTF8(obj->getProperty("access_token").toString().toRawUTF8(), (int)obj->getProperty("access_token").toString().getNumBytesAsUTF8()),
                                            juce::String::fromUTF8(obj->getProperty("refresh_token").toString().toRawUTF8(), (int)obj->getProperty("refresh_token").toString().getNumBytesAsUTF8())
                                        );
                                        if (callback) callback(true);
                                    } else {
                                        DBG("R2OneDriveProvider instance no longer valid or not of expected type during refresh token UI callback.");
                                        if (callback) callback(false);
                                    }
                                });
                                return;
                            }
                        }
                    }
                }
                DBG("Refresh token failed. Status: " + juce::String(statusCode) + ", Response: " + responseString);
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
            } else {
                DBG("Refresh token failed: Stream creation returned nullptr.");
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
            }
        } else {
            DBG("R2OneDriveProvider instance invalid during refreshAccessToken thread. Aborting.");
            juce::MessageManager::callAsync([callback](){ if (callback) callback(false); });
        }
    });
}

// makeAPIRequest (string body)
void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method, const juce::StringPairArray& headers, const juce::String& body, std::function<void(bool, int, const juce::var&)> callback)
{
    // Removed juce::ScopedLock
    if (!isTokenValid())
    {
        auto self = shared_from_this();
        // Removed sl.release();
        refreshAccessToken([this, self, endpoint, method, headers, body, callback](bool success) {
            if (success) this->makeAPIRequest(endpoint, method, headers, body, callback);
            else juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 401, juce::var()); });
        });
        return;
    }

    DBG("R2OneDriveProvider::makeAPIRequest (string body): " + method + " " + endpoint);
    auto selfWeak = std::weak_ptr<R2CloudStorageProvider>(shared_from_this());
    
    // Explicitly copy strings
    juce::String accessTokenCopy = accessToken; // Copy from member
    juce::String endpointCopy = endpoint;
    juce::String methodCopy = method;
    juce::String bodyCopy = body;
    juce::StringPairArray headersCopy = headers; // StringPairArray handles its own copies of juce::String internally

    juce::Thread::launch([selfWeak, callback, accessTokenCopy, endpointCopy, methodCopy, bodyCopy, headersCopy] () mutable {
        DBG("makeAPIRequest (string body) thread started.");
        DBG("makeAPIRequest (string body) thread: accessTokenCopy len=" + juce::String(accessTokenCopy.length()) + ", endpointCopy=" + endpointCopy.substring(0, juce::jmin(endpointCopy.length(), 50)) + ", methodCopy=" + methodCopy + ", bodyCopy len=" + juce::String(bodyCopy.length()));

        if (auto selfShared = selfWeak.lock()) {
            juce::URL url(endpointCopy);
            
            if ((methodCopy == "POST" || methodCopy == "PUT" || methodCopy == "PATCH") && bodyCopy.isNotEmpty())
            {
                url = url.withPOSTData(bodyCopy);
            }

            juce::StringPairArray requestHeaders = headersCopy;
            requestHeaders.set("Authorization", "Bearer " + accessTokenCopy);

            juce::String headerString;
            for (int i = 0; i < requestHeaders.size(); ++i) headerString << requestHeaders.getAllKeys()[i] << ": " << requestHeaders.getAllValues()[i] << "\r\n";

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders(headerString)
                           .withConnectionTimeoutMs(30000)
                           .withHttpRequestCmd(methodCopy);
            
            DBG("makeAPIRequest (string body): Attempting to create input stream. URL: " + url.toString(true));
            std::unique_ptr<juce::InputStream> stream;
            try {
                stream = url.createInputStream(options);
            } catch (const std::exception& e) {
                DBG("Exception creating HTTP input stream (string body): " + juce::String(e.what()));
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 0, juce::var()); });
                return;
            } catch (...) {
                DBG("Unknown exception creating HTTP input stream (string body).");
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 0, juce::var()); });
                return;
            }

            if (stream) {
                DBG("makeAPIRequest (string body): Input stream created, reading response.");
                int statusCode = 0;
                if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get())) {
                    statusCode = webStream->getStatusCode();
                    DBG("makeAPIRequest (string body): HTTP Status Code: " + juce::String(statusCode));
                } else {
                    DBG("makeAPIRequest (string body): Could not cast stream to WebInputStream for status code.");
                }
                
                juce::String responseString = stream->readEntireStreamAsString();
                DBG("makeAPIRequest (string body): Raw Response length: " + juce::String(responseString.length()));
                juce::var responseJson = juce::JSON::parse(responseString);
                DBG("makeAPIRequest (string body): JSON parsed.");

                juce::MessageManager::callAsync([callback, statusCode, responseJson]() {
                    if (callback) callback(statusCode >= 200 && statusCode < 300, statusCode, responseJson);
                });
            } else {
                DBG("makeAPIRequest (string body): Stream creation returned nullptr.");
                juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 0, juce::var()); });
            }
        } else {
            DBG("R2OneDriveProvider instance invalid during makeAPIRequest (string body) thread. Aborting.");
            juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 0, juce::var()); });
        }
    });
}

// makeAPIRequest (MemoryBlock body)
void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method, const juce::StringPairArray& headers, const juce::MemoryBlock& body, std::function<void(bool, int, const juce::var&)> callback)
{
    // Removed juce::ScopedLock
    if (!isTokenValid())
    {
        auto self = shared_from_this();
        // Removed sl.release();
        refreshAccessToken([this, self, endpoint, method, headers, body, callback](bool success) {
            if (success) this->makeAPIRequest(endpoint, method, headers, body, callback);
            else juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 401, juce::var()); });
        });
        return;
    }

    DBG("R2OneDriveProvider::makeAPIRequest (MemoryBlock body): " + method + " " + endpoint);
    auto selfWeak = std::weak_ptr<R2CloudStorageProvider>(shared_from_this());
    
    // Explicitly copy strings
    juce::String accessTokenCopy = accessToken; // Copy from member
    juce::String endpointCopy = endpoint;
    juce::String methodCopy = method;
    juce::MemoryBlock bodyCopy = body; // MemoryBlock already performs deep copy
    juce::StringPairArray headersCopy = headers;

    juce::Thread::launch([selfWeak, callback, accessTokenCopy, endpointCopy, methodCopy, bodyCopy, headersCopy] () mutable {
        DBG("makeAPIRequest (MemoryBlock body) thread started.");
        DBG("makeAPIRequest (MemoryBlock body) thread: accessTokenCopy len=" + juce::String(accessTokenCopy.length()) + ", endpointCopy=" + endpointCopy.substring(0, juce::jmin(endpointCopy.length(), 50)) + ", methodCopy=" + methodCopy + ", bodyCopy len=" + juce::String(bodyCopy.getSize()));

        if (auto selfShared = selfWeak.lock()) {
            juce::URL url(endpointCopy);
            
            if ((methodCopy == "PUT" || methodCopy == "POST" || methodCopy == "PATCH") && bodyCopy.getSize() > 0)
            {
                url = url.withPOSTData(bodyCopy);
            }

            juce::StringPairArray requestHeaders = headersCopy;
            requestHeaders.set("Authorization", "Bearer " + accessTokenCopy);

            juce::String headerString;
            for (int i = 0; i < requestHeaders.size(); ++i) headerString << requestHeaders.getAllKeys()[i] << ": " << requestHeaders.getAllValues()[i] << "\r\n";

            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders(headerString)
                           .withConnectionTimeoutMs(30000)
                           .withHttpRequestCmd(methodCopy);
                           
            DBG("makeAPIRequest (MemoryBlock body): Attempting to create input stream. URL: " + url.toString(true));
            std::unique_ptr<juce::InputStream> stream;
            try {
                stream = url.createInputStream(options);
            } catch (const std::exception& e) {
                DBG("Exception creating HTTP input stream (MemoryBlock body): " + juce::String(e.what()));
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 0, juce::var()); });
                return;
            } catch (...) {
                DBG("Unknown exception creating HTTP input stream (MemoryBlock body).");
                juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 0, juce::var()); });
                return;
            }
            
            if (stream) {
                DBG("makeAPIRequest (MemoryBlock body): Input stream created, reading response.");
                int statusCode = 0;
                if (auto* webStream = dynamic_cast<juce::WebInputStream*>(stream.get())) {
                    statusCode = webStream->getStatusCode();
                    DBG("makeAPIRequest (MemoryBlock body): HTTP Status Code: " + juce::String(statusCode));
                } else {
                    DBG("makeAPIRequest (MemoryBlock body): Could not cast stream to WebInputStream for status code.");
                }

                juce::String responseString = stream->readEntireStreamAsString();
                DBG("makeAPIRequest (MemoryBlock body): Raw Response length: " + juce::String(responseString.length()));
                juce::var responseJson = juce::JSON::parse(responseString);
                DBG("makeAPIRequest (MemoryBlock body): JSON parsed.");

                juce::MessageManager::callAsync([callback, statusCode, responseJson]() {
                    if (callback) callback(statusCode >= 200 && statusCode < 300, statusCode, responseJson);
                });
            } else {
                DBG("makeAPIRequest (MemoryBlock body): Stream creation returned nullptr.");
                juce::MessageManager::callAsync([callback]() { if (callback) callback(false, 0, juce::var()); });
            }
        } else {
            DBG("R2OneDriveProvider instance invalid during makeAPIRequest (MemoryBlock body) thread. Aborting.");
            juce::MessageManager::callAsync([callback](){ if (callback) callback(false, 0, juce::var()); });
        }
    });
}

void R2OneDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    DBG("R2OneDriveProvider::downloadFileByPath called for: " + filePath);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(filePath, true);
    
    auto self = shared_from_this();
    juce::String filePathCopy = filePath;

    makeAPIRequest(endpoint, "GET", {}, juce::MemoryBlock(), [self, callback, filePathCopy](bool success, int statusCode, const juce::var& responseVar)
    {
        DBG(juce::String("downloadFileByPath API Request Callback received for: ") + filePathCopy + ", Success: " + juce::String(success ? "true" : "false"));
        if (!success || statusCode != 200) {
            DBG("Download file metadata failed for " + filePathCopy + ". Status: " + juce::String(statusCode) + " " + juce::JSON::toString(responseVar));
            if (callback) callback(false, {}, "File not found or metadata failed");
            return;
        }
        if (auto* obj = responseVar.getDynamicObject()) {
            juce::String downloadUrl = obj->getProperty("@microsoft.graph.downloadUrl").toString();
            if (downloadUrl.isEmpty()) {
                DBG("Download file metadata successful for " + filePathCopy + " but no download URL found.");
                if (callback) callback(false, {}, "Could not get download URL");
                return;
            }
            DBG("Download file metadata received for " + filePathCopy + ", URL: " + downloadUrl.substring(0, juce::jmin(downloadUrl.length(), 50)) + "..."); // Truncate URL for log
            
            juce::URL url(downloadUrl);
            auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withConnectionTimeoutMs(30000);
            
            juce::Thread::launch([url, options, callback, filePathCopy]() {
                DBG("Download content thread started for: " + filePathCopy);
                std::unique_ptr<juce::InputStream> stream;
                try {
                    stream = url.createInputStream(options);
                } catch (const std::exception& e) {
                    DBG("Exception creating download content input stream for " + filePathCopy + ": " + juce::String(e.what()));
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content (exception)"); });
                    return;
                } catch (...) {
                    DBG("Unknown exception creating download content input stream for " + filePathCopy + ".");
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content (unknown exception)"); });
                    return;
                }

                if (stream) {
                    DBG("Download content stream created for " + filePathCopy + ", reading data.");
                    juce::MemoryBlock downloadedData;
                    stream->readIntoMemoryBlock(downloadedData);
                    DBG("Download content read complete for " + filePathCopy + ", size: " + juce::String(downloadedData.getSize()));
                    juce::MessageManager::callAsync([callback, downloadedData, filePathCopy](){ if (callback) callback(true, downloadedData, ""); });
                } else {
                    DBG("Download file content failed for " + filePathCopy + ": Stream creation returned nullptr.");
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Failed to download file content (stream nullptr)"); });
                }
            });
        } else {
            DBG("Failed to parse file metadata for download of " + filePathCopy + ": " + juce::JSON::toString(responseVar));
            if (callback) callback(false, {}, "Failed to parse file metadata");
        }
    });
}

void R2OneDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    DBG("R2OneDriveProvider::uploadFileByPath called for: " + filePath);
    auto pathParts = juce::StringArray::fromTokens(filePath, "/", "");
    if (pathParts.isEmpty()) { if (callback) callback(false, "Invalid file path"); return; }
    auto fileName = pathParts[pathParts.size() - 1];
    pathParts.remove(pathParts.size() - 1);
    auto self = shared_from_this();
    juce::String fileNameCopy = fileName;
    juce::MemoryBlock dataCopy = data;

    createFolderPath(pathParts, "root", 0, [this, self, fileNameCopy, dataCopy, callback](bool success, juce::String parentId) {
        if (success) {
            DBG("createFolderPath successful for " + fileNameCopy + ", parentId: " + parentId);
            this->uploadToFolder(fileNameCopy, parentId, dataCopy, callback);
        }
        else {
            DBG("createFolderPath failed for " + fileNameCopy + ": " + parentId);
            if (callback) callback(false, "Failed to create folder path for " + fileNameCopy + ": " + parentId);
        }
    });
}

void R2OneDriveProvider::createFolderPath(const juce::StringArray& folderPath, const juce::String& parentId, int pathIndex, std::function<void(bool, juce::String)> callback)
{
    DBG("R2OneDriveProvider::createFolderPath: Path Index: " + juce::String(pathIndex) + ", Parent ID: " + parentId);
    if (pathIndex >= folderPath.size())
    {
        DBG("createFolderPath: All folders created/found. Final Parent ID: " + parentId);
        if (callback) callback(true, parentId);
        return;
    }

    auto folderName = folderPath[pathIndex];
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + "/children";
    endpoint += "?$filter=name eq '" + juce::URL::addEscapeChars(folderName, true) + "'";

    auto selfWeak = std::weak_ptr<R2CloudStorageProvider>(shared_from_this());
    juce::String folderNameCopy = folderName;
    juce::String parentIdCopy = parentId;
    juce::StringArray folderPathCopy = folderPath;

    makeAPIRequest(endpoint, "GET", {}, juce::MemoryBlock(), [selfWeak, folderPathCopy, pathIndex, folderNameCopy, parentIdCopy, callback](bool success, int statusCode, const juce::var& responseVar)
    {
        DBG(juce::String("createFolderPath API Request Callback for '") + folderNameCopy + "', Success: " + juce::String(success ? "true" : "false") + ", Status: " + juce::String(statusCode));
        if (auto providerPtr = dynamic_cast<R2OneDriveProvider*>(selfWeak.lock().get())) {
            if (!success || statusCode != 200) { // Check status code for success as well
                DBG("Failed to check folder existence '" + folderNameCopy + "'. Status: " + juce::String(statusCode) + ", Response: " + juce::JSON::toString(responseVar));
                // If it's 404 (Not Found), then we attempt to create it. Other errors are actual failures.
                if (statusCode == 404) {
                    DBG("Folder '" + folderNameCopy + "' not found (404), attempting to create.");
                    juce::String createEndpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentIdCopy + "/children";
                    
                    auto* folderObj = new juce::DynamicObject();
                    folderObj->setProperty("name", folderNameCopy);
                    folderObj->setProperty("folder", new juce::DynamicObject());
                    folderObj->setProperty("@microsoft.graph.conflictBehavior", "fail");

                    juce::StringPairArray headers;
                    headers.set("Content-Type", "application/json");

                    juce::String jsonBody = juce::JSON::toString(juce::var(folderObj));
                    juce::MemoryBlock jsonBodyData(jsonBody.toRawUTF8(), jsonBody.getNumBytesAsUTF8());

                    providerPtr->makeAPIRequest(createEndpoint, "POST", headers, jsonBodyData, [selfWeak, folderPathCopy, pathIndex, folderNameCopy, callback](bool createSuccess, int createStatusCode, const juce::var& createResponseVar)
                    {
                        if (auto innerProviderPtr = dynamic_cast<R2OneDriveProvider*>(selfWeak.lock().get())) {
                            if (!createSuccess) {
                                juce::String errorMessage = "Failed to create folder '" + folderNameCopy + "'. Status: " + juce::String(createStatusCode) + ", Response: " + juce::JSON::toString(createResponseVar);
                                DBG(errorMessage);
                                if (callback) callback(false, errorMessage);
                                return;
                            }
                            
                            juce::String newId = createResponseVar.getDynamicObject()->getProperty("id").toString();
                            DBG("Folder '" + folderNameCopy + "' created with ID: " + newId);
                            innerProviderPtr->createFolderPath(folderPathCopy, newId, pathIndex + 1, callback);
                        } else {
                            DBG("R2OneDriveProvider instance no longer valid during createFolderPath inner callback.");
                            if (callback) callback(false, "Internal error: Provider invalid during folder creation.");
                        }
                    });
                } else { // It's a real error checking for folder existence
                    if (callback) callback(false, "Failed to check folder existence with status: " + juce::String(statusCode));
                }
                return;
            }
            
            auto value = responseVar.getDynamicObject()->getProperty("value");
            if (value.isArray() && value.getArray()->size() > 0)
            {
                juce::String existingId = value[0].getDynamicObject()->getProperty("id").toString();
                DBG("Folder '" + folderNameCopy + "' already exists with ID: " + existingId);
                providerPtr->createFolderPath(folderPathCopy, existingId, pathIndex + 1, callback);
            }
            else // Folder not found, but API returned 200 (empty array). Treat as not found and create.
            {
                DBG("Folder '" + folderNameCopy + "' not found by filter (empty array), attempting to create.");
                juce::String createEndpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentIdCopy + "/children";
                
                auto* folderObj = new juce::DynamicObject();
                folderObj->setProperty("name", folderNameCopy);
                folderObj->setProperty("folder", new juce::DynamicObject());
                folderObj->setProperty("@microsoft.graph.conflictBehavior", "fail");

                juce::StringPairArray headers;
                headers.set("Content-Type", "application/json");

                juce::String jsonBody = juce::JSON::toString(juce::var(folderObj));
                juce::MemoryBlock jsonBodyData(jsonBody.toRawUTF8(), jsonBody.getNumBytesAsUTF8());

                providerPtr->makeAPIRequest(createEndpoint, "POST", headers, jsonBodyData, [selfWeak, folderPathCopy, pathIndex, folderNameCopy, callback](bool createSuccess, int createStatusCode, const juce::var& createResponseVar)
                {
                    if (auto innerProviderPtr = dynamic_cast<R2OneDriveProvider*>(selfWeak.lock().get())) {
                        if (!createSuccess) {
                            juce::String errorMessage = "Failed to create folder '" + folderNameCopy + "'. Status: " + juce::String(createStatusCode) + ", Response: " + juce::JSON::toString(createResponseVar);
                            DBG(errorMessage);
                            if (callback) callback(false, errorMessage);
                            return;
                        }
                        
                        juce::String newId = createResponseVar.getDynamicObject()->getProperty("id").toString();
                            DBG("Folder '" + folderNameCopy + "' created with ID: " + newId);
                        innerProviderPtr->createFolderPath(folderPathCopy, newId, pathIndex + 1, callback);
                    } else {
                        DBG("R2OneDriveProvider instance no longer valid during createFolderPath inner callback (create fallback).");
                        if (callback) callback(false, "Internal error: Provider invalid during folder creation fallback.");
                    }
                });
            }
        } else {
            DBG("R2OneDriveProvider instance no longer valid during createFolderPath callback.");
            if (callback) callback(false, "Internal error: Provider invalid during folder existence check.");
        }
    });
}


void R2OneDriveProvider::uploadToFolder(const juce::String& fileName, const juce::String& parentId, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    DBG("R2OneDriveProvider::uploadToFolder called for: " + fileName + " in " + parentId);
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + ":/" + juce::URL::addEscapeChars(fileName, true) + ":/content";
    juce::StringPairArray headers; headers.set("Content-Type", "application/octet-stream");
    
    juce::String fileNameCopy = fileName;
    juce::String parentIdCopy = parentId;
    juce::MemoryBlock dataCopy = data;

    makeAPIRequest(endpoint, "PUT", headers, dataCopy, [callback, fileNameCopy, parentIdCopy](bool success, int statusCode, const juce::var& response) {
        if (success) {
            DBG("Upload successful for file: " + fileNameCopy + " to " + parentIdCopy);
            if (callback) callback(true, "Upload successful");
        } else {
            juce::String errorMessage = "Upload failed for file: " + fileNameCopy + " to " + parentIdCopy + ". Status: " + juce::String(statusCode) + ", Response: " + juce::JSON::toString(response);
            DBG(errorMessage);
            if (callback) callback(false, errorMessage);
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
    juce::String jsonContent = juce::JSON::toString(juce::var(tokenData));

    // Write to file
    if (! tokenFile.replaceWithText(jsonContent))
    {
        DBG("ERROR: Failed to write token file: " + tokenFile.getFullPathName());
        return;
    }
    DBG("Tokens saved to: " + tokenFile.getFullPathName());

    // Verify written content by reading it back
    if (tokenFile.existsAsFile())
    {
        juce::String verifyContent = tokenFile.loadFileAsString();
        juce::var verifyParsed = juce::JSON::parse(verifyContent);
        if (verifyParsed.isObject())
        {
            auto* verifyObj = verifyParsed.getDynamicObject();
            juce::String verifyAccessToken = verifyObj->getProperty("access_token").toString();
            juce::String verifyRefreshToken = verifyObj->getProperty("refresh_token").toString();

            // FIX: Check for complete match of saved tokens
            if (verifyAccessToken == accessToken && verifyRefreshToken == refreshToken)
            {
                DBG("Token file verification SUCCESS: Content matches!");
            }
            else
            {
                DBG("ERROR: Token file verification FAILED: Content mismatch after save!");
                DBG("Saved Access Token Length: " + juce::String(accessToken.length()) + ", Read Access Token Length: " + juce::String(verifyAccessToken.length()));
                DBG("Saved Refresh Token Length: " + juce::String(refreshToken.length()) + ", Read Refresh Token Length: " + juce::String(verifyRefreshToken.length()));
                DBG("Saved Access Token (first 50): " + accessToken.substring(0, juce::jmin(accessToken.length(), 50)));
                DBG("Read Access Token (first 50): " + verifyAccessToken.substring(0, juce::jmin(verifyAccessToken.length(), 50)));
                DBG("Saved Refresh Token (first 50): " + refreshToken.substring(0, juce::jmin(refreshToken.length(), 50)));
                DBG("Read Refresh Token (first 50): " + verifyRefreshToken.substring(0, juce::jmin(verifyRefreshToken.length(), 50)));
            }
        }
        else
        {
            DBG("ERROR: Token file verification FAILED: Could not parse JSON from saved file or file not found after save.");
        }
    }
    else
    {
        DBG("ERROR: Token file verification FAILED: File does not exist after save operation.");
    }
}

bool R2OneDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
    if (!tokenFile.existsAsFile()) {
        DBG("Token file does not exist: " + tokenFile.getFullPathName());
        return false;
    }
    juce::String fileContent = tokenFile.loadFileAsString();
    juce::var parsed = juce::JSON::parse(fileContent);
    if(auto* obj = parsed.getDynamicObject()) {
        accessToken = obj->getProperty("access_token").toString();
        refreshToken = obj->getProperty("refresh_token").toString();
        tokenExpiry = juce::Time(static_cast<juce::int64>(obj->getProperty("token_expiry")));
        DBG(juce::String("Tokens loaded. Access Token Empty: ") + (accessToken.isEmpty() ? juce::String("true") : juce::String("false")) + ", Refresh Token Empty: " + (refreshToken.isEmpty() ? juce::String("true") : juce::String("false")));
        if (refreshToken.isNotEmpty()) { currentStatus = Status::Authenticated; return true; }
    }
    DBG("Failed to load tokens: JSON object is invalid or properties missing.");
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
