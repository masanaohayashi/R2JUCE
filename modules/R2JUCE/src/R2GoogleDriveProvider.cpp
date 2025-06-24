#include "R2GoogleDriveProvider.h"

namespace r2juce {

// The OAuth2Handler class from your original file.
class R2GoogleDriveProvider::OAuth2Handler : public juce::Component
{
public:
    OAuth2Handler(R2GoogleDriveProvider& parent) : provider(parent)
    {
        juce::WebBrowserComponent::Options options;
        
        options = options.withNativeIntegrationEnabled(true);
        
        options = options.withNativeFunction("authComplete",
            [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion)
            {
                DBG("authComplete called with " + juce::String(args.size()) + " args");
                if (args.size() > 0)
                {
                    auto authCode = args[0].toString();
                    if (authCode.isNotEmpty())
                    {
                        DBG("Auth code received: " + authCode);
                        handleAuthSuccess(authCode);
                    }
                    else
                    {
                        handleAuthError("No auth code received");
                    }
                }
                else
                {
                    handleAuthError("Invalid auth response");
                }
                completion(juce::var());
            });
        
        options = options.withNativeFunction("authError",
            [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion)
            {
                juce::String error = "Authentication failed";
                if (args.size() > 0)
                {
                    error = args[0].toString();
                }
                DBG("Auth error: " + error);
                handleAuthError(error);
                completion(juce::var());
            });
        
        options = options.withEventListener("urlChanged",
            [this](const juce::var& eventData)
            {
                if (eventData.isObject())
                {
                    auto* obj = eventData.getDynamicObject();
                    if (obj && obj->hasProperty("url"))
                    {
                        auto url = obj->getProperty("url").toString();
                        DBG("URL changed to: " + url);
                        checkAuthCallback(url);
                    }
                }
            });
        
        juce::String monitoringScript = R"(
            let lastUrl = window.location.href;
            function checkUrlChange() {
                const currentUrl = window.location.href;
                if (currentUrl !== lastUrl) {
                    lastUrl = currentUrl;
                    if (window.__JUCE__ && window.__JUCE__.backend) {
                        window.__JUCE__.backend.emitEvent('urlChanged', { url: currentUrl });
                    }
                    if (currentUrl.includes('localhost:8080/callback')) {
                        const params = new URLSearchParams(window.location.search);
                        const code = params.get('code');
                        const error = params.get('error');
                        if (code) {
                            if (window.authComplete) { window.authComplete(code); }
                        } else if (error) {
                            const errorDesc = params.get('error_description') || error;
                            if (window.authError) { window.authError(errorDesc); }
                        }
                    }
                }
            }
            setInterval(checkUrlChange, 1000);
            if (document.readyState === 'loading') {
                document.addEventListener('DOMContentLoaded', checkUrlChange);
            } else {
                checkUrlChange();
            }
        )";
        
        options = options.withUserScript(monitoringScript);
        options = options.withUserAgent("CloudDoc/1.0 (JUCE WebBrowser)");
        
        webBrowser = std::make_unique<juce::WebBrowserComponent>(options);
        addAndMakeVisible(*webBrowser);
    }
    
    void startAuthentication(const juce::String& clientId, R2CloudStorageProvider::AuthCallback callback)
    {
        authCallback = callback;
        currentClientId = clientId;
        redirectUri = "http://localhost:8080/callback";
        
        juce::String authUrl = "https://accounts.google.com/o/oauth2/v2/auth?";
        authUrl += "client_id=" + juce::URL::addEscapeChars(clientId, false);
        authUrl += "&redirect_uri=" + juce::URL::addEscapeChars(redirectUri, false);
        authUrl += "&response_type=code";
        authUrl += "&scope=" + juce::URL::addEscapeChars("https://www.googleapis.com/auth/drive", false);
        authUrl += "&access_type=offline";
        authUrl += "&prompt=consent";
        authUrl += "&state=" + generateStateParameter();
        
        DBG("Starting authentication with URL: " + authUrl);
        webBrowser->goToURL(authUrl);
    }
    
    void resized() override { webBrowser->setBounds(getLocalBounds()); }

private:
    juce::String generateStateParameter()
    {
        auto randomValue = juce::Random::getSystemRandom().nextInt64();
        stateParameter = juce::String::toHexString(static_cast<juce::int64>(randomValue));
        return stateParameter;
    }
    
    void checkAuthCallback(const juce::String& url)
    {
        if (url.startsWith(redirectUri))
        {
            auto authCode = extractAuthCode(url);
            if (extractState(url) != stateParameter && stateParameter.isNotEmpty()) { handleAuthError("Invalid state parameter - possible CSRF attack"); return; }
            if (authCode.isNotEmpty()) { handleAuthSuccess(authCode); }
            else if (auto error = extractError(url); error.isNotEmpty()) { handleAuthError(error); }
            else { handleAuthError("No authorization code or error found in callback URL"); }
        }
        else if (url.contains("error"))
        {
            if (auto error = extractError(url); error.isNotEmpty()) { handleAuthError(error); }
        }
    }
    
    void handleAuthSuccess(const juce::String& authCode)
    {
        DBG("Authentication successful");
        showSuccessPage();
        provider.exchangeAuthCodeForTokens(authCode, [cb = authCallback](bool success, const juce::String& errorMessage) {
            if (cb) juce::MessageManager::callAsync([cb, success, errorMessage]() { cb(success, errorMessage); });
        });
    }
    
    void handleAuthError(const juce::String& error)
    {
        DBG("Authentication error: " + error);
        showErrorPage(error);
        if (authCallback)
        {
            juce::MessageManager::callAsync([cb = authCallback, error]() { cb(false, error); });
            authCallback = nullptr;
        }
    }
    
    juce::String extractURLParameter(const juce::String& url, const juce::String& paramName)
    {
        if (auto queryStart = url.indexOf("?"); queryStart != -1)
        {
            auto params = juce::StringArray::fromTokens(url.substring(queryStart + 1), "&", "");
            for (const auto& param : params)
                if (param.startsWith(paramName + "="))
                    return juce::URL::removeEscapeChars(param.substring((paramName + "=").length()));
        }
        return {};
    }

    juce::String extractAuthCode(const juce::String& url) { return extractURLParameter(url, "code"); }
    juce::String extractState(const juce::String& url) { return extractURLParameter(url, "state"); }
    juce::String extractError(const juce::String& url)
    {
        auto error = extractURLParameter(url, "error");
        if (error.isNotEmpty())
        {
            auto errorDesc = extractURLParameter(url, "error_description");
            return errorDesc.isNotEmpty() ? error + ": " + errorDesc : error;
        }
        return {};
    }
    
    void showSuccessPage()
    {
        juce::String successHTML = R"(omitted)";
        webBrowser->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(successHTML, false));
    }
    
    void showErrorPage(const juce::String& error)
    {
        juce::String errorHTML = R"(omitted)" + error + R"(omitted)";
        webBrowser->goToURL("data:text/html;charset=utf-8," + juce::URL::addEscapeChars(errorHTML, false));
    }

    R2GoogleDriveProvider& provider;
    std::unique_ptr<juce::WebBrowserComponent> webBrowser;
    R2CloudStorageProvider::AuthCallback authCallback;
    juce::String redirectUri;
    juce::String currentClientId;
    juce::String stateParameter;
};

// Class Implementation starts here
R2GoogleDriveProvider::R2GoogleDriveProvider()
{
    currentStatus = Status::NotAuthenticated;
    loadTokens();
}

void R2GoogleDriveProvider::setClientCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    this->clientId = clientId;
    this->clientSecret = clientSecret;
}

void R2GoogleDriveProvider::authenticate(AuthCallback callback)
{
    if (clientId.isEmpty() || clientSecret.isEmpty())
    {
        if (callback) callback(false, "Client credentials not set");
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
            if (success)
            {
                currentStatus = Status::Authenticated;
                if (callback) callback(true, "Authenticated with refresh token");
            }
            else
            {
                currentStatus = Status::NotAuthenticated;
                if (callback) callback(false, "device flow authentication required");
            }
        });
        return;
    }
    
    currentStatus = Status::NotAuthenticated;
    if (callback) callback(false, "device flow authentication required");
}

void R2GoogleDriveProvider::startNewAuthFlow(AuthCallback callback)
{
    currentStatus = Status::Authenticating;
    if (!oauth2Handler)
        oauth2Handler = std::make_unique<OAuth2Handler>(*this);
    oauth2Handler->startAuthentication(clientId, callback);
}

void R2GoogleDriveProvider::exchangeAuthCodeForTokens(const juce::String& authCode, std::function<void(bool, juce::String)> callback)
{
    juce::String postData = "grant_type=authorization_code";
    postData += "&code=" + juce::URL::addEscapeChars(authCode, false);
    postData += "&redirect_uri=" + juce::URL::addEscapeChars("http://localhost:8080/callback", false);
    postData += "&client_id=" + juce::URL::addEscapeChars(clientId, false);
    postData += "&client_secret=" + juce::URL::addEscapeChars(clientSecret, false);
    
    juce::URL tokenUrl("https://oauth2.googleapis.com/token");
    tokenUrl = tokenUrl.withPOSTData(postData);
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/x-www-form-urlencoded");
    headers.set("Accept", "application/json");
    
    juce::Thread::launch([this, tokenUrl, headers, callback]()
    {
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
        {
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                headerString += "\r\n";
        }

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                        .withExtraHeaders(headerString)
                        .withConnectionTimeoutMs(30000)
                        .withNumRedirectsToFollow(5);
        
        if (auto stream = tokenUrl.createInputStream(options))
        {
            auto response = stream->readEntireStreamAsString();
            juce::MessageManager::callAsync([this, response, callback]() { parseTokenResponse(response, callback); });
        }
        else
        {
            juce::MessageManager::callAsync([callback]() { if (callback) callback(false, "Failed to create HTTP request"); });
        }
    });
}

void R2GoogleDriveProvider::setTokens(const juce::String& accessToken, const juce::String& refreshToken)
{
    this->accessToken = accessToken;
    if(refreshToken.isNotEmpty()) this->refreshToken = refreshToken;
    this->tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(3600);
    this->currentStatus = Status::Authenticated;
    saveTokens();
}

void R2GoogleDriveProvider::uploadFileByPath(const juce::String& filePath, const juce::MemoryBlock& data, FileOperationCallback callback)
{
    uploadFile(filePath, data, "path", callback);
}

void R2GoogleDriveProvider::downloadFileByPath(const juce::String& filePath, DownloadCallback callback)
{
    auto pathParts = juce::StringArray::fromTokens(filePath, "/", "");
    if (pathParts.isEmpty())
    {
        if (callback) callback(false, {}, "Invalid file path");
        return;
    }
    findFileByPath(pathParts, "root", 0, callback);
}

void R2GoogleDriveProvider::cancelAuthentication()
{
    if (oauth2Handler)
        oauth2Handler.reset();
    currentStatus = Status::NotAuthenticated;
}

void R2GoogleDriveProvider::parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> callback)
{
    try
    {
        auto json = juce::JSON::parse(response);
        if (auto* obj = json.getDynamicObject())
        {
            if (obj->hasProperty("access_token"))
            {
                accessToken = obj->getProperty("access_token").toString();
                if (obj->hasProperty("refresh_token")) refreshToken = obj->getProperty("refresh_token").toString();
                if (obj->hasProperty("expires_in")) tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(static_cast<int>(obj->getProperty("expires_in")) - 300);
                
                currentStatus = Status::Authenticated;
                saveTokens();
                if (callback) callback(true, "Authentication successful");
                return;
            }
            else if (obj->hasProperty("error"))
            {
                currentStatus = Status::Error;
                auto error = obj->getProperty("error").toString();
                auto errorDesc = obj->getProperty("error_description").toString();
                if (callback) callback(false, errorDesc.isNotEmpty() ? error + ": " + errorDesc : error);
                return;
            }
        }
    }
    catch (...) { }

    currentStatus = Status::Error;
    if (callback) callback(false, "Invalid token response");
}

void R2GoogleDriveProvider::findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId,
                                          int pathIndex, DownloadCallback callback)
{
    if (pathIndex >= pathParts.size())
    {
        if (callback) callback(false, {}, "Path not found");
        return;
    }
    
    auto targetName = pathParts[pathIndex];
    bool isLastPart = (pathIndex == pathParts.size() - 1);
    
    listFiles(currentFolderId, [this, pathParts, pathIndex, targetName, isLastPart, callback]
             (bool success, juce::Array<FileInfo> files, juce::String errorMessage)
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
                if (isLastPart && !file.isFolder) { downloadFile(file.id, callback); return; }
                else if (!isLastPart && file.isFolder) { findFileByPath(pathParts, file.id, pathIndex + 1, callback); return; }
            }
        }
        
        if (callback) callback(false, {}, "Path not found: " + targetName);
    });
}

void R2GoogleDriveProvider::uploadToFolder(const juce::String& fileName, const juce::MemoryBlock& data,
                                          const juce::String& folderId, FileOperationCallback callback)
{
    juce::String targetFolderId = folderId.isEmpty() ? "root" : folderId;
    juce::String query = "'" + targetFolderId + "' in parents and name = '" + fileName + "' and mimeType != 'application/vnd.google-apps.folder' and trashed = false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false) + "&fields=files(id,name,trashed)";
    
    makeAPIRequest(endpoint, "GET", {}, "", [this, fileName, data, targetFolderId, callback]
                  (bool success, const juce::String& response)
    {
        if (!success)
        {
            if (callback) callback(false, "Failed to check existing files: " + response);
            return;
        }
        
        juce::String existingFileId;
        try
        {
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject())
                if (auto* filesArray = obj->getProperty("files").getArray())
                    if (filesArray->size() > 0)
                        if (auto* file = filesArray->getUnchecked(0).getDynamicObject())
                            if (!file->getProperty("trashed"))
                                existingFileId = file->getProperty("id").toString();
        }
        catch (...) { }
        
        if (existingFileId.isNotEmpty())
            updateExistingFile(existingFileId, data, callback);
        else
            uploadNewFile(fileName, data, targetFolderId, callback);
    });
}

void R2GoogleDriveProvider::uploadWithData(const juce::String& endpoint, const juce::String& boundary,
                                          const juce::MemoryBlock& fullData, FileOperationCallback callback)
{
    juce::URL url(endpoint);
    juce::StringPairArray headers;
    headers.set("Authorization", "Bearer " + accessToken);
    headers.set("Content-Type", "multipart/related; boundary=" + boundary);
    headers.set("Content-Length", juce::String(fullData.getSize()));

    juce::String headerString;
    for (int i=0; i<headers.size(); ++i) headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i] + "\r\n";

    url = url.withPOSTData(fullData);
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                    .withExtraHeaders(headerString)
                    .withConnectionTimeoutMs(30000);
    
    if (auto stream = url.createInputStream(options))
    {
        auto response = stream->readEntireStreamAsString();
        juce::MessageManager::callAsync([callback, response]() {
            try
            {
                auto json = juce::JSON::parse(response);
                if (json.isObject() && json.getDynamicObject()->hasProperty("id")) {
                    if(callback) callback(true, "File uploaded successfully");
                } else {
                    if(callback) callback(false, "Upload failed: " + response);
                }
            } catch(...) { if(callback) callback(false, "Failed to parse upload response"); }
        });
    }
    else
    {
        // THIS IS THE FIX for the assertion.
        // The callback is now wrapped in callAsync to ensure it runs on the message thread.
        juce::MessageManager::callAsync([callback]() {
            if(callback) callback(false, "Failed to create upload request");
        });
    }
}

void R2GoogleDriveProvider::updateExistingFile(const juce::String& fileId, const juce::MemoryBlock& data,
                                               FileOperationCallback callback)
{
    DBG("Updating existing file with ID: " + fileId);
    
    juce::Thread::launch([this, fileId, data, callback]()
    {
        if (!isTokenValid())
        {
            refreshAccessToken([this, fileId, data, callback](bool refreshSuccess)
            {
                if (!refreshSuccess) {
                    juce::MessageManager::callAsync([callback](){ if (callback) callback(false, "Authentication required"); });
                    return;
                }
                updateExistingFile(fileId, data, callback);
            });
            return;
        }
        
        juce::String endpoint = "https://www.googleapis.com/upload/drive/v3/files/" + fileId + "?uploadType=media";
        juce::URL url(endpoint);
        
        juce::StringPairArray headers;
        headers.set("Authorization", "Bearer " + accessToken);
        headers.set("Content-Type", "application/octet-stream");
        headers.set("Content-Length", juce::String(data.getSize()));
        
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i] + "\r\n";
        
        url = url.withPOSTData(data);
        
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                        .withExtraHeaders(headerString)
                        .withConnectionTimeoutMs(30000)
                        .withHttpRequestCmd("PATCH");
        
        if (auto stream = url.createInputStream(options))
        {
            auto response = stream->readEntireStreamAsString();
            juce::MessageManager::callAsync([callback, response]() {
                try {
                    auto json = juce::JSON::parse(response);
                    if (json.isObject() && json.getDynamicObject()->hasProperty("id")) {
                        if (callback) callback(true, "File updated successfully");
                    } else {
                        if (callback) callback(false, "Update failed: " + response);
                    }
                } catch (...) { if (callback) callback(false, "Failed to parse update response"); }
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback]() { if (callback) callback(false, "Failed to create update request"); });
        }
    });
}


void R2GoogleDriveProvider::uploadNewFile(const juce::String& fileName, const juce::MemoryBlock& data,
                                         const juce::String& folderId, FileOperationCallback callback)
{
    DBG("Uploading new file: " + fileName);
    
    juce::String boundary = "----formdata-juce-" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64());
    
    // Use JUCE's JSON classes for safety. This part is an improvement.
    auto* metadataObject = new juce::DynamicObject();
    metadataObject->setProperty("name", fileName);
    if (folderId.isNotEmpty() && folderId != "root")
    {
        juce::Array<juce::var> parents;
        parents.add(folderId);
        metadataObject->setProperty("parents", parents);
    }
    juce::String metadata = juce::JSON::toString(juce::var(metadataObject));

    //
    // === FIX ===
    // Revert the multipart body construction to your original, working implementation.
    // My previous refactoring of this part was faulty.
    //
    juce::String multipartBody;
    multipartBody += "--" + boundary + "\r\n";
    multipartBody += "Content-Type: application/json; charset=UTF-8\r\n\r\n";
    multipartBody += metadata + "\r\n";
    multipartBody += "--" + boundary + "\r\n";
    multipartBody += "Content-Type: application/octet-stream\r\n\r\n";
    
    juce::MemoryBlock fullData;
    fullData.append(multipartBody.toRawUTF8(), multipartBody.getNumBytesAsUTF8());
    fullData.append(data.getData(), data.getSize());
    
    juce::String endBoundary = "\r\n--" + boundary + "--\r\n";
    fullData.append(endBoundary.toRawUTF8(), endBoundary.getNumBytesAsUTF8());
    
    // === END OF FIX ===
    
    juce::String endpoint = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";
    
    juce::Thread::launch([this, endpoint, boundary, fullData, callback]()
    {
        if (!isTokenValid())
        {
            refreshAccessToken([this, endpoint, boundary, fullData, callback](bool refreshSuccess)
            {
                if (!refreshSuccess) { if (callback) callback(false, "Authentication required"); return; }
                juce::Thread::launch([this, endpoint, boundary, fullData, callback]() { uploadWithData(endpoint, boundary, fullData, callback); });
            });
            return;
        }
        uploadWithData(endpoint, boundary, fullData, callback);
    });
}

void R2GoogleDriveProvider::createFolderPath(const juce::StringArray& folderPath, const juce::String& parentFolderId,
                                            int pathIndex, std::function<void(bool, juce::String, juce::String)> callback)
{
    if (pathIndex >= folderPath.size())
    {
        if (callback) callback(true, parentFolderId, "");
        return;
    }
    
    auto folderName = folderPath[pathIndex];
    juce::String query = "'" + parentFolderId + "' in parents and name = '" + folderName + "' and mimeType = 'application/vnd.google-apps.folder' and trashed = false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false) + "&fields=files(id,name,mimeType)";
    
    makeAPIRequest(endpoint, "GET", {}, "", [this, folderPath, folderName, parentFolderId, pathIndex, callback]
                  (bool success, const juce::String& response)
    {
        if (!success) { if (callback) callback(false, "", "Failed to check existing folder"); return; }
        
        try
        {
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject()) {
                if (obj->hasProperty("files")) {
                    if (auto* filesArray = obj->getProperty("files").getArray()) {
                        if (filesArray->size() > 0) {
                            if (auto* fileObj = filesArray->getUnchecked(0).getDynamicObject()) {
                                juce::String existingFolderId = fileObj->getProperty("id").toString();
                                createFolderPath(folderPath, existingFolderId, pathIndex + 1, callback);
                                return;
                            }
                        }
                    }
                }
            }
        } catch(...) { }
        
        juce::String metadata = "{ \"name\": \"" + folderName + "\", \"mimeType\": \"application/vnd.google-apps.folder\"";
        if (parentFolderId.isNotEmpty()) metadata += ", \"parents\": [\"" + parentFolderId + "\"]";
        metadata += "}";
        
        juce::StringPairArray headers; headers.set("Content-Type", "application/json");
        makeAPIRequest("https://www.googleapis.com/drive/v3/files?fields=id,name", "POST", headers, metadata, [this, folderPath, pathIndex, callback]
                      (bool createSuccess, const juce::String& createResponse)
        {
            if (!createSuccess) { if (callback) callback(false, "", "Failed to create folder: " + createResponse); return; }
            try
            {
                auto json = juce::JSON::parse(createResponse);
                if (auto* obj = json.getDynamicObject())
                    if (obj->hasProperty("id"))
                    {
                        createFolderPath(folderPath, obj->getProperty("id").toString(), pathIndex + 1, callback);
                        return;
                    }
            } catch (...) { }
            if (callback) callback(false, "", "Failed to create folder - invalid response");
        });
    });
}

void R2GoogleDriveProvider::signOut()
{
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = juce::Time();
    currentStatus = Status::NotAuthenticated;
    if (auto tokenFile = getTokenFile(); tokenFile.exists())
        tokenFile.deleteFile();
}

R2CloudStorageProvider::Status R2GoogleDriveProvider::getAuthStatus() const { return currentStatus; }
juce::String R2GoogleDriveProvider::getDisplayName() const { return "Google Drive"; }
bool R2GoogleDriveProvider::isTokenValid() const { return accessToken.isNotEmpty() && (tokenExpiry == juce::Time() || juce::Time::getCurrentTime() < tokenExpiry); }

void R2GoogleDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    if (refreshToken.isEmpty()) { if (callback) callback(false); return; }
    
    juce::String postData = "grant_type=refresh_token";
    postData += "&refresh_token=" + juce::URL::addEscapeChars(refreshToken, false);
    postData += "&client_id=" + juce::URL::addEscapeChars(clientId, false);
    postData += "&client_secret=" + juce::URL::addEscapeChars(clientSecret, false);
    
    juce::URL tokenUrl("https://oauth2.googleapis.com/token");
    tokenUrl = tokenUrl.withPOSTData(postData);
    
    juce::StringPairArray headers; headers.set("Content-Type", "application/x-www-form-urlencoded"); headers.set("Accept", "application/json");
    
    juce::Thread::launch([this, tokenUrl, headers, callback]()
    {
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i) headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i] + "\r\n";
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withExtraHeaders(headerString).withConnectionTimeoutMs(30000);
        
        if (auto stream = tokenUrl.createInputStream(options))
            juce::MessageManager::callAsync([this, response = stream->readEntireStreamAsString(), callback]() { parseTokenResponse(response, [callback](bool s, const juce::String&){ if(callback) callback(s); }); });
        else
            juce::MessageManager::callAsync([callback](){ if(callback) callback(false); });
    });
}

void R2GoogleDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method,
                                          const juce::StringPairArray& headers, const juce::String& postData,
                                          std::function<void(bool, juce::String)> callback)
{
    if (!isTokenValid())
    {
        refreshAccessToken([this, endpoint, method, headers, postData, callback](bool refreshSuccess)
        {
            if (refreshSuccess)
                makeAPIRequest(endpoint, method, headers, postData, callback);
            else
            {
                currentStatus = Status::NotAuthenticated;
                if (callback) callback(false, "Authentication required");
            }
        });
        return;
    }
    
    juce::Thread::launch([this, endpoint, method, headers, postData, callback]()
    {
        juce::URL url(endpoint);
        juce::StringPairArray requestHeaders = headers;
        requestHeaders.set("Authorization", "Bearer " + accessToken);
        requestHeaders.set("Accept", "application/json");
        
        if (method == "POST" && postData.isNotEmpty())
        {
            url = url.withPOSTData(postData);
            if (!requestHeaders.containsKey("Content-Type"))
                requestHeaders.set("Content-Type", "application/json");
        }
        
        juce::String headerString;
        for (int i = 0; i < requestHeaders.size(); ++i)
            headerString += requestHeaders.getAllKeys()[i] + ": " + requestHeaders.getAllValues()[i] + "\r\n";

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withExtraHeaders(headerString).withConnectionTimeoutMs(30000).withHttpRequestCmd(method);
        
        auto stream = url.createInputStream(options);
        auto streamPtr = stream.release();
        juce::MessageManager::callAsync([streamPtr, callback]()
        {
            std::unique_ptr<juce::InputStream> stream(streamPtr);
            if (stream) {
                if (callback) {
                    callback(true, stream->readEntireStreamAsString());
                }
                else {
                    if (callback) callback(false, "Request failed");
                }
            }
        });
    });
}

void R2GoogleDriveProvider::listFiles(const juce::String& folderId, FileListCallback callback)
{
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,mimeType,modifiedTime,size)";
    juce::String query = "'" + (folderId.isNotEmpty() ? folderId : "root") + "' in parents and trashed=false";
    endpoint += "&q=" + juce::URL::addEscapeChars(query, false);
    
    makeAPIRequest(endpoint, "GET", {}, "", [callback](bool success, const juce::String& response)
    {
        if (!success) { if (callback) callback(false, {}, "Failed to list files"); return; }
        
        juce::Array<FileInfo> files;
        try
        {
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject())
                if (auto* filesArray = obj->getProperty("files").getArray())
                    for (auto& fileJson : *filesArray)
                        if (auto* fileObj = fileJson.getDynamicObject())
                        {
                            FileInfo info;
                            info.id = fileObj->getProperty("id").toString();
                            info.name = fileObj->getProperty("name").toString();
                            info.mimeType = fileObj->getProperty("mimeType").toString();
                            info.isFolder = info.mimeType == "application/vnd.google-apps.folder";
                            if (fileObj->hasProperty("size")) info.size = fileObj->getProperty("size").toString().getLargeIntValue(); else info.size = 0;
                            if (fileObj->hasProperty("modifiedTime")) info.modifiedTime = juce::Time::fromISO8601(fileObj->getProperty("modifiedTime").toString());
                            files.add(info);
                        }
            if (callback) callback(true, files, "");
        } catch (...) { if (callback) callback(false, {}, "Failed to parse file list response"); }
    });
}

void R2GoogleDriveProvider::uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                                      const juce::String& folderId, FileOperationCallback callback)
{
    if (folderId == "path")
    {
        auto pathParts = juce::StringArray::fromTokens(fileName, "/", "");
        if (pathParts.size() <= 1)
        {
            uploadToFolder(fileName, data, "root", callback);
        }
        else
        {
            auto folderPath = pathParts;
            folderPath.remove(folderPath.size() - 1);
            auto actualFileName = pathParts[pathParts.size() - 1];
            
            createFolderPath(folderPath, "root", 0, [this, actualFileName, data, callback] (bool success, juce::String newFolderId, juce::String errorMessage)
            {
                if (success) uploadToFolder(actualFileName, data, newFolderId, callback);
                else if (callback) callback(false, "Failed to create folder path: " + errorMessage);
            });
        }
    }
    else
    {
        uploadToFolder(fileName, data, folderId, callback);
    }
}

void R2GoogleDriveProvider::downloadFile(const juce::String& fileId, DownloadCallback callback)
{
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files/" + fileId + "?alt=media";
    
    juce::Thread::launch([this, endpoint, callback]()
    {
        if (!isTokenValid()) { juce::MessageManager::callAsync([callback](){ if (callback) callback(false, {}, "Authentication required"); }); return; }
        
        juce::URL url(endpoint);
        juce::StringPairArray headers; headers.set("Authorization", "Bearer " + accessToken);
        juce::String headerString;
        for (int i=0; i<headers.size(); ++i) headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i] + "\r\n";

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress).withExtraHeaders(headerString).withConnectionTimeoutMs(30000);
        
        auto stream = url.createInputStream(options);
        auto streamPtr = stream.release();
        juce::MessageManager::callAsync([streamPtr, callback]()
        {
            std::unique_ptr<juce::InputStream> stream(streamPtr);
            if (stream)
            {
                juce::MemoryBlock data;
                stream->readIntoMemoryBlock(data);
                if (callback) callback(true, data, "");
            }
            else { if (callback) callback(false, {}, "Failed to download file"); }
        });
    });
}

void R2GoogleDriveProvider::deleteFile(const juce::String& fileId, FileOperationCallback callback)
{
    makeAPIRequest("https://www.googleapis.com/drive/v3/files/" + fileId, "DELETE", {}, "", [callback](bool success, const juce::String&){
        if (callback) callback(success, success ? "File deleted successfully" : "Failed to delete file");
    });
}

void R2GoogleDriveProvider::createFolder(const juce::String& folderName, const juce::String& parentId,
                                        FileOperationCallback callback)
{
    juce::String metadata = "{ \"name\": \"" + folderName + "\", \"mimeType\": \"application/vnd.google-apps.folder\"";
    if (parentId.isNotEmpty() && parentId != "root")
        metadata += ",\n  \"parents\": [\"" + parentId + "\"]";
    metadata += "}";
    
    juce::StringPairArray headers; headers.set("Content-Type", "application/json");
    makeAPIRequest("https://www.googleapis.com/drive/v3/files", "POST", headers, metadata, [callback](bool success, const juce::String& response)
    {
        if (!success) { if (callback) callback(false, "Failed to create folder - API request failed"); return; }
        
        try
        {
            auto json = juce::JSON::parse(response);
            if (auto* obj = json.getDynamicObject())
            {
                if (obj->hasProperty("id")) { if (callback) callback(true, "Folder created successfully"); return; }
                else if (obj->hasProperty("error"))
                {
                    if(auto* errObj = obj->getProperty("error").getDynamicObject())
                        if(errObj->hasProperty("message"))
                            { if(callback) callback(false, "Failed to create folder: " + errObj->getProperty("message").toString()); return; }
                }
            }
        } catch (...) { }
        if (callback) callback(false, "Failed to parse folder creation response");
    });
}

void R2GoogleDriveProvider::saveTokens()
{
    auto tokenFile = getTokenFile();
    juce::DynamicObject::Ptr tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", static_cast<juce::int64>(tokenExpiry.toMilliseconds()));
    tokenFile.replaceWithText(juce::JSON::toString(juce::var(tokenData.get())));
}

bool R2GoogleDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
    if (!tokenFile.exists()) return false;
    
    auto tokenData = juce::JSON::parse(tokenFile.loadFileAsString());
    if (auto* obj = tokenData.getDynamicObject())
    {
        accessToken = obj->getProperty("access_token").toString();
        refreshToken = obj->getProperty("refresh_token").toString();
        tokenExpiry = juce::Time(static_cast<juce::int64>(obj->getProperty("token_expiry")));
        
        if (isTokenValid()) currentStatus = Status::Authenticated;
        else if (refreshToken.isNotEmpty()) DBG("Access token expired, but refresh token available");
        return true;
    }
    return false;
}

juce::File R2GoogleDriveProvider::getTokenFile() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto cloudDocDir = appDataDir.getChildFile("CloudDoc");
    if (!cloudDocDir.exists()) cloudDocDir.createDirectory();
    return cloudDocDir.getChildFile("google_drive_tokens.json");
}

} // namespace r2juce
