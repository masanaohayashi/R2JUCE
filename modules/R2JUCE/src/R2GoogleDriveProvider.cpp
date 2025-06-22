#include "R2GoogleDriveProvider.h"

namespace r2juce {

R2GoogleDriveProvider::OAuth2Handler::OAuth2Handler(R2GoogleDriveProvider& parent) : provider(parent)
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
        // Script to monitor URL changes
        let lastUrl = window.location.href;
        
        function checkUrlChange() {
            const currentUrl = window.location.href;
            if (currentUrl !== lastUrl) {
                lastUrl = currentUrl;
                
                // Notify JUCE backend of URL change
                if (window.__JUCE__ && window.__JUCE__.backend) {
                    window.__JUCE__.backend.emitEvent('urlChanged', { url: currentUrl });
                }
                
                // Check if this is an auth callback URL
                if (currentUrl.includes('localhost:8080/callback')) {
                    const params = new URLSearchParams(window.location.search);
                    const code = params.get('code');
                    const error = params.get('error');
                    
                    if (code) {
                        if (window.authComplete) {
                            window.authComplete(code);
                        }
                    } else if (error) {
                        const errorDesc = params.get('error_description') || error;
                        if (window.authError) {
                            window.authError(errorDesc);
                        }
                    }
                }
            }
        }
        
        // Check URL periodically
        setInterval(checkUrlChange, 1000);
        
        // Also check when page loading is complete
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

void R2GoogleDriveProvider::OAuth2Handler::startAuthentication(const juce::String& clientId, R2CloudStorageProvider::AuthCallback callback)
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

void R2GoogleDriveProvider::OAuth2Handler::resized()
{
    webBrowser->setBounds(getLocalBounds());
}

    
juce::String R2GoogleDriveProvider::OAuth2Handler::generateStateParameter()
{
    auto randomValue = juce::Random::getSystemRandom().nextInt64();
    stateParameter = juce::String::toHexString(static_cast<juce::int64>(randomValue));
    return stateParameter;
}
    
void R2GoogleDriveProvider::OAuth2Handler::checkAuthCallback(const juce::String& url)
{
    DBG("Checking URL: " + url);
    
    if (url.startsWith(redirectUri))
    {
        DBG("Redirect URI matched, extracting auth code");
        
        auto authCode = extractAuthCode(url);
        auto state = extractState(url);
        auto error = extractError(url);
        
        if (state != stateParameter && stateParameter.isNotEmpty())
        {
            handleAuthError("Invalid state parameter - possible CSRF attack");
            return;
        }
        
        if (authCode.isNotEmpty())
        {
            handleAuthSuccess(authCode);
        }
        else if (error.isNotEmpty())
        {
            handleAuthError(error);
        }
        else
        {
            handleAuthError("No authorization code or error found in callback URL");
        }
    }
    else if (url.contains("error"))
    {
        auto error = extractError(url);
        if (error.isNotEmpty())
        {
            handleAuthError(error);
        }
    }
}

void R2GoogleDriveProvider::OAuth2Handler::handleAuthSuccess(const juce::String& authCode)
{
    DBG("Authentication successful");
    showSuccessPage();

    auto currentCallback = authCallback;

    provider.exchangeAuthCodeForTokens(authCode, [this, currentCallback](bool success, const juce::String& errorMessage)
        {
            if (currentCallback)
            {
                juce::MessageManager::callAsync([currentCallback, success, errorMessage]()
                    {
                        currentCallback(success, errorMessage);
                    });
            }
        });
}

void R2GoogleDriveProvider::OAuth2Handler::handleAuthError(const juce::String& error)
{
    DBG("Authentication error: " + error);
    showErrorPage(error);

    auto currentCallback = authCallback;

    if (currentCallback)
    {
        juce::MessageManager::callAsync([currentCallback, error]()
            {
                currentCallback(false, error);
            });

        authCallback = nullptr;
    }
}

juce::String R2GoogleDriveProvider::OAuth2Handler::extractURLParameter(const juce::String& url, const juce::String& paramName)
{
    auto queryStart = url.indexOf("?");
    if (queryStart == -1) return {};

    auto query = url.substring(queryStart + 1);
    auto params = juce::StringArray::fromTokens(query, "&", "");

    juce::String searchParam = paramName + "=";
    for (const auto& param : params)
    {
        if (param.startsWith(searchParam))
        {
            return juce::URL::removeEscapeChars(param.substring(searchParam.length()));
        }
    }
    return {};
}

juce::String R2GoogleDriveProvider::OAuth2Handler::extractAuthCode(const juce::String& url)
{
    return extractURLParameter(url, "code");
}

juce::String R2GoogleDriveProvider::OAuth2Handler::extractState(const juce::String& url)
{
    return extractURLParameter(url, "state");
}

juce::String R2GoogleDriveProvider::OAuth2Handler::extractError(const juce::String& url)
{
    auto error = extractURLParameter(url, "error");
    if (error.isNotEmpty())
    {
        auto errorDesc = extractURLParameter(url, "error_description");
        if (errorDesc.isNotEmpty())
        {
            return error + ": " + errorDesc;
        }
        return error;
    }
    return {};
}

void R2GoogleDriveProvider::OAuth2Handler::showSuccessPage()
{
    juce::String successHTML = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>認証成功</title>
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            text-align: center; 
            padding: 50px; 
            background: linear-gradient(135deg, #4CAF50, #45a049); 
            color: white; 
            margin: 0;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: rgba(255,255,255,0.15); 
            padding: 50px; 
            border-radius: 25px; 
            box-shadow: 0 8px 32px rgba(0,0,0,0.2);
            max-width: 500px;
            width: 100%;
        }
        .checkmark { 
            font-size: 5em; 
            color: #ffffff; 
            text-shadow: 2px 2px 8px rgba(0,0,0,0.3);
            margin-bottom: 20px;
        }
        h1 {
            font-size: 2.5em;
            margin: 20px 0;
            font-weight: 300;
        }
        p {
            font-size: 1.2em;
            margin: 15px 0;
            opacity: 0.9;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="checkmark">✓</div>
        <h1>認証成功！</h1>
        <p>Google Driveへの接続が完了しました。</p>
        <p>このウィンドウは閉じて構いません。</p>
    </div>
</body>
</html>
        )";
        
    webBrowser->goToURL("data:text/html;charset=utf-8," +
                       juce::URL::addEscapeChars(successHTML, false));
}

void R2GoogleDriveProvider::OAuth2Handler::showErrorPage(const juce::String& error)
{
    juce::String errorHTML = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>認証エラー</title>
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            text-align: center; 
            padding: 50px; 
            background: linear-gradient(135deg, #f44336, #d32f2f); 
            color: white; 
            margin: 0;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
        }
        .container { 
            background: rgba(255,255,255,0.15); 
            padding: 50px; 
            border-radius: 25px; 
            box-shadow: 0 8px 32px rgba(0,0,0,0.2);
            max-width: 500px;
            width: 100%;
        }
        .error-icon { 
            font-size: 5em; 
            color: #ffffff; 
            text-shadow: 2px 2px 8px rgba(0,0,0,0.3);
            margin-bottom: 20px;
        }
        h1 {
            font-size: 2.5em;
            margin: 20px 0;
            font-weight: 300;
        }
        p {
            font-size: 1.2em;
            margin: 15px 0;
            opacity: 0.9;
        }
        .error-detail {
            background: rgba(255,255,255,0.1);
            padding: 20px;
            border-radius: 10px;
            margin: 20px 0;
            font-family: monospace;
            word-break: break-word;
            font-size: 0.9em;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="error-icon">✗</div>
        <h1>認証失敗</h1>
        <p>Google Drive認証中にエラーが発生しました。</p>
        <div class="error-detail">)" + error + R"(</div>
        <p>ウィンドウを閉じて再試行してください。</p>
    </div>
</body>
</html>
        )";
        
    webBrowser->goToURL("data:text/html;charset=utf-8," +
                       juce::URL::addEscapeChars(errorHTML, false));
}

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
    DBG("R2GoogleDriveProvider::authenticate() called");
    
    if (clientId.isEmpty() || clientSecret.isEmpty())
    {
        DBG("Client credentials not set - clientId: " + clientId + ", clientSecret: " + clientSecret);
        if (callback)
            callback(false, "Client credentials not set");
        return;
    }
    
    DBG("Client credentials are set");
    
    if (isTokenValid())
    {
        DBG("Token is valid, already authenticated");
        currentStatus = Status::Authenticated;
        if (callback)
            callback(true, "Already authenticated");
        return;
    }
    
    DBG("Token not valid, checking refresh token");
    
    if (refreshToken.isNotEmpty())
    {
        DBG("Refresh token available, trying to refresh");
        currentStatus = Status::Authenticating;
        refreshAccessToken([this, callback](bool success)
        {
            if (success)
            {
                currentStatus = Status::Authenticated;
                if (callback)
                    callback(true, "Authenticated with refresh token");
            }
            else
            {
                currentStatus = Status::NotAuthenticated;
                if (callback)
                    callback(false, "device flow authentication required");
            }
        });
        return;
    }
    
    DBG("No refresh token, device flow authentication required");
    
    currentStatus = Status::NotAuthenticated;
    if (callback)
        callback(false, "device flow authentication required");
}

void R2GoogleDriveProvider::startNewAuthFlow(AuthCallback callback)
{
    currentStatus = Status::Authenticating;
    
    if (!oauth2Handler)
    {
        oauth2Handler = std::make_unique<OAuth2Handler>(*this);
    }
    
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
        
        auto stream = tokenUrl.createInputStream(options);
        
        if (stream != nullptr)
        {
            auto response = stream->readEntireStreamAsString();
            
            juce::MessageManager::callAsync([this, response, callback]()
            {
                parseTokenResponse(response, callback);
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, "Failed to create HTTP request");
            });
        }
    });
}

void R2GoogleDriveProvider::setTokens(const juce::String& accessToken, const juce::String& refreshToken)
{
    this->accessToken = accessToken;
    this->refreshToken = refreshToken;
    
    this->tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::hours(1);
    
    this->currentStatus = Status::Authenticated;
    saveTokens();
    
    DBG("Tokens set successfully");
}

void R2GoogleDriveProvider::downloadFileWithPath(const juce::String& filePath, DownloadCallback callback)
{
    auto pathParts = juce::StringArray::fromTokens(filePath, "/", "");
    
    if (pathParts.isEmpty())
    {
        if (callback)
            callback(false, {}, "Invalid file path");
        return;
    }
    
    findFileByPath(pathParts, "root", 0, callback);
}

void R2GoogleDriveProvider::cancelAuthentication()
{
    DBG("R2GoogleDriveProvider::cancelAuthentication() called");
    
    if (oauth2Handler)
    {
        oauth2Handler.reset();
    }
    
    currentStatus = Status::NotAuthenticated;
}

void R2GoogleDriveProvider::parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> callback)
{
    try
    {
        auto json = juce::JSON::parse(response);
        
        if (json.isObject())
        {
            auto* obj = json.getDynamicObject();
            
            if (obj->hasProperty("access_token"))
            {
                accessToken = obj->getProperty("access_token").toString();
                
                if (obj->hasProperty("refresh_token"))
                {
                    refreshToken = obj->getProperty("refresh_token").toString();
                }
                
                if (obj->hasProperty("expires_in"))
                {
                    auto expiresIn = static_cast<int>(obj->getProperty("expires_in"));
                    tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(expiresIn - 300);
                }
                
                currentStatus = Status::Authenticated;
                saveTokens();
                
                if (callback)
                    callback(true, "Authentication successful");
                return;
            }
            else if (obj->hasProperty("error"))
            {
                auto error = obj->getProperty("error").toString();
                auto errorDesc = obj->getProperty("error_description").toString();
                juce::String errorMessage = error;
                if (errorDesc.isNotEmpty())
                    errorMessage += ": " + errorDesc;
                    
                currentStatus = Status::Error;
                if (callback)
                    callback(false, errorMessage);
                return;
            }
        }
        
        currentStatus = Status::Error;
        if (callback)
            callback(false, "Invalid token response");
    }
    catch (...)
    {
        currentStatus = Status::Error;
        if (callback)
            callback(false, "Failed to parse token response");
    }
}

void R2GoogleDriveProvider::findFileByPath(const juce::StringArray& pathParts, const juce::String& currentFolderId,
                                          int pathIndex, DownloadCallback callback)
{
    if (pathIndex >= pathParts.size())
    {
        if (callback)
            callback(false, {}, "Path not found");
        return;
    }
    
    auto targetName = pathParts[pathIndex];
    bool isLastPart = (pathIndex == pathParts.size() - 1);
    
    listFiles(currentFolderId, [this, pathParts, pathIndex, targetName, isLastPart, callback]
             (bool success, juce::Array<FileInfo> files, juce::String errorMessage)
    {
        if (!success)
        {
            if (callback)
                callback(false, {}, "Failed to list files: " + errorMessage);
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
                else if (!isLastPart && file.isFolder)
                {
                    findFileByPath(pathParts, file.id, pathIndex + 1, callback);
                    return;
                }
            }
        }
        
        if (callback)
            callback(false, {}, "Path not found: " + targetName);
    });
}

void R2GoogleDriveProvider::uploadToFolder(const juce::String& fileName, const juce::MemoryBlock& data,
                                          const juce::String& folderId, FileOperationCallback callback)
{
    DBG("uploadToFolder - fileName: " + fileName + ", folderId: " + folderId + ", data size: " + juce::String(data.getSize()));
    
    juce::String targetFolderId = folderId.isEmpty() ? "root" : folderId;
    
    juce::String query = "'" + targetFolderId + "' in parents and name = '" + fileName + "' and mimeType != 'application/vnd.google-apps.folder' and trashed = false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false);
    endpoint += "&fields=files(id,name,trashed)";
    
    makeAPIRequest(endpoint, "GET", {}, "", [this, fileName, data, targetFolderId, callback]
                  (bool success, const juce::String& response)
    {
        if (!success)
        {
            DBG("Failed to list files: " + response);
            if (callback)
                callback(false, "Failed to check existing files: " + response);
            return;
        }
        
        juce::String existingFileId;
        try
        {
            auto json = juce::JSON::parse(response);
            if (json.isObject())
            {
                auto* obj = json.getDynamicObject();
                if (obj->hasProperty("files"))
                {
                    auto filesArray = obj->getProperty("files");
                    if (filesArray.isArray())
                    {
                        auto* array = filesArray.getArray();
                        if (array->size() > 0)
                        {
                            auto fileObj = array->getUnchecked(0);
                            if (fileObj.isObject())
                            {
                                auto* file = fileObj.getDynamicObject();
                                bool isTrashed = false;
                                if (file->hasProperty("trashed"))
                                {
                                    isTrashed = file->getProperty("trashed");
                                }
                                
                                if (!isTrashed)
                                {
                                    existingFileId = file->getProperty("id").toString();
                                    DBG("Found existing file with ID: " + existingFileId);
                                }
                                else
                                {
                                    DBG("Found file but it's in trash, ignoring");
                                }
                            }
                        }
                    }
                }
            }
        }
        catch (...)
        {
            DBG("Failed to parse file list response");
        }
        
        if (existingFileId.isNotEmpty())
        {
            updateExistingFile(existingFileId, data, callback);
        }
        else
        {
            uploadNewFile(fileName, data, targetFolderId, callback);
        }
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
    for (int i = 0; i < headers.size(); ++i)
    {
        headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
        if (i < headers.size() - 1)
            headerString += "\r\n";
    }
    
    url = url.withPOSTData(fullData);
    
    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                    .withExtraHeaders(headerString)
                    .withConnectionTimeoutMs(30000);
    
    auto stream = url.createInputStream(options);
    
    if (stream != nullptr)
    {
        auto response = stream->readEntireStreamAsString();
        DBG("Upload response: " + response);
        
        try
        {
            auto json = juce::JSON::parse(response);
            if (json.isObject())
            {
                auto* obj = json.getDynamicObject();
                if (obj->hasProperty("id"))
                {
                    DBG("Upload successful, file ID: " + obj->getProperty("id").toString());
                    juce::MessageManager::callAsync([callback]()
                    {
                        if (callback)
                            callback(true, "File uploaded successfully");
                    });
                    return;
                }
            }
        }
        catch (...)
        {
            DBG("Failed to parse upload response");
        }
        
        juce::MessageManager::callAsync([callback, response]()
        {
            if (callback)
                callback(false, "Upload failed: " + response);
        });
    }
    else
    {
        DBG("Failed to create upload stream");
        juce::MessageManager::callAsync([callback]()
        {
            if (callback)
                callback(false, "Failed to create upload request");
        });
    }
}

void R2GoogleDriveProvider::updateExistingFile(const juce::String& fileId, const juce::MemoryBlock& data,
                                               FileOperationCallback callback)
{
    DBG("Updating existing file with ID: " + fileId);
    
    juce::String endpoint = "https://www.googleapis.com/upload/drive/v3/files/" + fileId + "?uploadType=media";
    
    juce::Thread::launch([this, endpoint, data, callback]()
    {
        if (!isTokenValid())
        {
            DBG("Token is not valid, refreshing...");
            refreshAccessToken([this, endpoint, data, callback](bool refreshSuccess)
            {
                if (!refreshSuccess)
                {
                    DBG("Failed to refresh token");
                    juce::MessageManager::callAsync([callback]()
                    {
                        if (callback)
                            callback(false, "Authentication required");
                    });
                    return;
                }
                
                updateExistingFile(endpoint, data, callback);
            });
            return;
        }
        
        juce::URL url(endpoint);
        
        juce::StringPairArray headers;
        headers.set("Authorization", "Bearer " + accessToken);
        headers.set("Content-Type", "application/octet-stream");
        headers.set("Content-Length", juce::String(data.getSize()));
        
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
        {
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                headerString += "\r\n";
        }
        
        url = url.withPOSTData(data);
        
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                        .withExtraHeaders(headerString)
                        .withConnectionTimeoutMs(30000)
                        .withHttpRequestCmd("PATCH");
        
        auto stream = url.createInputStream(options);
        
        if (stream != nullptr)
        {
            auto response = stream->readEntireStreamAsString();
            DBG("Update response: " + response);
            
            try
            {
                auto json = juce::JSON::parse(response);
                if (json.isObject())
                {
                    auto* obj = json.getDynamicObject();
                    if (obj->hasProperty("id"))
                    {
                        DBG("Update successful, file ID: " + obj->getProperty("id").toString());
                        juce::MessageManager::callAsync([callback]()
                        {
                            if (callback)
                                callback(true, "File updated successfully");
                        });
                        return;
                    }
                }
            }
            catch (...)
            {
                DBG("Failed to parse update response");
            }
            
            juce::MessageManager::callAsync([callback, response]()
            {
                if (callback)
                    callback(false, "Update failed: " + response);
            });
        }
        else
        {
            DBG("Failed to create update stream");
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, "Failed to create update request");
            });
        }
    });
}

void R2GoogleDriveProvider::uploadNewFile(const juce::String& fileName, const juce::MemoryBlock& data,
                                         const juce::String& folderId, FileOperationCallback callback)
{
    DBG("Uploading new file: " + fileName);
    
    juce::String boundary = "----formdata-juce-" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt64());
    
    juce::String metadata = "{\n";
    metadata += "  \"name\": \"" + fileName + "\"";
    if (folderId.isNotEmpty() && folderId != "root")
    {
        metadata += ",\n  \"parents\": [\"" + folderId + "\"]";
    }
    metadata += "\n}";
    
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
    
    juce::String endpoint = "https://www.googleapis.com/upload/drive/v3/files?uploadType=multipart";
    
    juce::Thread::launch([this, endpoint, boundary, fullData, callback]()
    {
        if (!isTokenValid())
        {
            DBG("Token is not valid, refreshing...");
            refreshAccessToken([this, endpoint, boundary, fullData, callback](bool refreshSuccess)
            {
                if (!refreshSuccess)
                {
                    DBG("Failed to refresh token");
                    juce::MessageManager::callAsync([callback]()
                    {
                        if (callback)
                            callback(false, "Authentication required");
                    });
                    return;
                }
                
                juce::Thread::launch([this, endpoint, boundary, fullData, callback]()
                {
                    uploadWithData(endpoint, boundary, fullData, callback);
                });
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
        DBG("Folder path creation complete. Final folder ID: " + parentFolderId);
        if (callback)
            callback(true, parentFolderId, "");
        return;
    }
    
    auto folderName = folderPath[pathIndex];
    DBG("Creating/checking folder: " + folderName + " in parent: " + parentFolderId);
    
    juce::String queryParentId = parentFolderId;
    if (parentFolderId == "root")
    {

    }
    
    juce::String query = "'" + queryParentId + "' in parents and name = '" + folderName + "' and mimeType = 'application/vnd.google-apps.folder' and trashed = false";
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?q=" + juce::URL::addEscapeChars(query, false);
    endpoint += "&fields=files(id,name,mimeType)";
    
    makeAPIRequest(endpoint, "GET", {}, "", [this, folderPath, folderName, parentFolderId, pathIndex, callback]
                  (bool success, const juce::String& response)
    {
        if (!success)
        {
            DBG("Failed to check existing folder: " + response);
            if (callback)
                callback(false, "", "Failed to check existing folder");
            return;
        }
        
        try
        {
            auto json = juce::JSON::parse(response);
            if (json.isObject())
            {
                auto* obj = json.getDynamicObject();
                if (obj->hasProperty("files"))
                {
                    auto filesArray = obj->getProperty("files");
                    if (filesArray.isArray())
                    {
                        auto* array = filesArray.getArray();
                        if (array->size() > 0)
                        {
                            auto fileObj = array->getUnchecked(0);
                            if (fileObj.isObject())
                            {
                                auto* file = fileObj.getDynamicObject();
                                juce::String existingFolderId = file->getProperty("id").toString();
                                DBG("Folder already exists with ID: " + existingFolderId);
                                
                                createFolderPath(folderPath, existingFolderId, pathIndex + 1, callback);
                                return;
                            }
                        }
                    }
                }
            }
        }
        catch (...)
        {
            DBG("Failed to parse folder check response");
        }
        
        DBG("Folder does not exist, creating: " + folderName);
        
        juce::String metadata = "{\n";
        metadata += "  \"name\": \"" + folderName + "\",\n";
        metadata += "  \"mimeType\": \"application/vnd.google-apps.folder\"";
        if (parentFolderId.isNotEmpty())
        {
            metadata += ",\n  \"parents\": [\"" + parentFolderId + "\"]";
        }
        metadata += "\n}";
        
        DBG("Creating folder with metadata: " + metadata);
        
        juce::String createEndpoint = "https://www.googleapis.com/drive/v3/files?fields=id,name";
        
        juce::StringPairArray headers;
        headers.set("Content-Type", "application/json");
        
        makeAPIRequest(createEndpoint, "POST", headers, metadata, [this, folderPath, pathIndex, callback]
                      (bool createSuccess, const juce::String& createResponse)
        {
            if (!createSuccess)
            {
                DBG("Failed to create folder: " + createResponse);
                if (callback)
                    callback(false, "", "Failed to create folder: " + createResponse);
                return;
            }
            
            try
            {
                auto json = juce::JSON::parse(createResponse);
                if (json.isObject())
                {
                    auto* obj = json.getDynamicObject();
                    if (obj->hasProperty("id"))
                    {
                        juce::String newFolderId = obj->getProperty("id").toString();
                        DBG("Folder created successfully with ID: " + newFolderId);
                        
                        createFolderPath(folderPath, newFolderId, pathIndex + 1, callback);
                        return;
                    }
                }
            }
            catch (...)
            {
                DBG("Failed to parse folder creation response");
            }
            
            if (callback)
                callback(false, "", "Failed to create folder - invalid response");
        });
    });
}

void R2GoogleDriveProvider::signOut()
{
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = juce::Time();
    currentStatus = Status::NotAuthenticated;
    
    auto tokenFile = getTokenFile();
    if (tokenFile.exists())
    {
        tokenFile.deleteFile();
    }
}

R2CloudStorageProvider::Status R2GoogleDriveProvider::getAuthStatus() const
{
    return currentStatus;
}

juce::String R2GoogleDriveProvider::getDisplayName() const
{
    return "Google Drive";
}

bool R2GoogleDriveProvider::isTokenValid() const
{
    return accessToken.isNotEmpty() &&
           (tokenExpiry == juce::Time() || juce::Time::getCurrentTime() < tokenExpiry);
}

void R2GoogleDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    if (refreshToken.isEmpty())
    {
        if (callback)
            callback(false);
        return;
    }
    
    juce::String postData = "grant_type=refresh_token";
    postData += "&refresh_token=" + juce::URL::addEscapeChars(refreshToken, false);
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
                        .withConnectionTimeoutMs(30000);
        
        auto stream = tokenUrl.createInputStream(options);
        
        if (stream != nullptr)
        {
            auto response = stream->readEntireStreamAsString();
            
            juce::MessageManager::callAsync([this, response, callback]()
            {
                parseTokenResponse(response, [callback](bool success, const juce::String&)
                {
                    if (callback)
                        callback(success);
                });
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false);
            });
        }
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
            {
                makeAPIRequest(endpoint, method, headers, postData, callback);
            }
            else
            {
                currentStatus = Status::NotAuthenticated;
                if (callback)
                    callback(false, "Authentication required");
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
            {
                requestHeaders.set("Content-Type", "application/json");
            }
        }
        
        juce::String headerString;
        for (int i = 0; i < requestHeaders.size(); ++i)
        {
            headerString += requestHeaders.getAllKeys()[i] + ": " + requestHeaders.getAllValues()[i];
            if (i < requestHeaders.size() - 1)
                headerString += "\r\n";
        }

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                        .withExtraHeaders(headerString)
                        .withConnectionTimeoutMs(30000)
                        .withHttpRequestCmd(method);
        
        auto stream = url.createInputStream(options);
        
        auto streamPtr = stream.release();
        juce::MessageManager::callAsync([streamPtr, callback]()
        {
            std::unique_ptr<juce::InputStream> stream(streamPtr);
            if (stream != nullptr)
            {
                auto response = stream->readEntireStreamAsString();
                if (callback)
                    callback(true, response);
            }
            else
            {
                if (callback)
                    callback(false, "Request failed");
            }
        });
    });
}

void R2GoogleDriveProvider::listFiles(const juce::String& folderId, FileListCallback callback)
{
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files?fields=files(id,name,mimeType,modifiedTime,size)";
    
    juce::String query;
    if (folderId.isNotEmpty() && folderId != "root")
    {
        query = "'" + folderId + "' in parents and trashed=false";
    }
    else
    {
        query = "'root' in parents and trashed=false";
    }
    endpoint += "&q=" + juce::URL::addEscapeChars(query, false);
    
    makeAPIRequest(endpoint, "GET", {}, "", [callback](bool success, const juce::String& response)
    {
        if (!success)
        {
            if (callback)
                callback(false, {}, "Failed to list files");
            return;
        }
        
        juce::Array<FileInfo> files;
        
        try
        {
            auto json = juce::JSON::parse(response);
            if (json.isObject())
            {
                auto* obj = json.getDynamicObject();
                if (obj->hasProperty("files"))
                {
                    auto filesArray = obj->getProperty("files");
                    if (filesArray.isArray())
                    {
                        auto* array = filesArray.getArray();
                        for (int i = 0; i < array->size(); ++i)
                        {
                            auto fileJson = array->getUnchecked(i);
                            if (fileJson.isObject())
                            {
                                auto* fileObj = fileJson.getDynamicObject();
                                
                                FileInfo fileInfo;
                                fileInfo.id = fileObj->getProperty("id").toString();
                                fileInfo.name = fileObj->getProperty("name").toString();
                                fileInfo.mimeType = fileObj->getProperty("mimeType").toString();
                                fileInfo.isFolder = fileInfo.mimeType == "application/vnd.google-apps.folder";
                                
                                if (fileObj->hasProperty("size"))
                                {
                                    auto sizeVar = fileObj->getProperty("size");
                                    if (sizeVar.isString())
                                    {
                                        fileInfo.size = sizeVar.toString().getLargeIntValue();
                                    }
                                    else if (sizeVar.isInt())
                                    {
                                        fileInfo.size = static_cast<int64_t>(static_cast<int>(sizeVar));
                                    }
                                    else if (sizeVar.isInt64())
                                    {
                                        fileInfo.size = static_cast<int64_t>(sizeVar.operator juce::int64());
                                    }
                                    else if (sizeVar.isDouble())
                                    {
                                        fileInfo.size = static_cast<int64_t>(static_cast<double>(sizeVar));
                                    }
                                    else
                                    {
                                        fileInfo.size = 0;
                                    }
                                }
                                else
                                {
                                    fileInfo.size = 0;
                                }
                                
                                if (fileObj->hasProperty("modifiedTime"))
                                {
                                    auto timeStr = fileObj->getProperty("modifiedTime").toString();
                                    fileInfo.modifiedTime = juce::Time::fromISO8601(timeStr);
                                }
                                
                                files.add(fileInfo);
                            }
                        }
                    }
                }
            }
            
            if (callback)
                callback(true, files, "");
        }
        catch (...)
        {
            if (callback)
                callback(false, {}, "Failed to parse file list response");
        }
    });
}

void R2GoogleDriveProvider::uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                                      const juce::String& folderId, FileOperationCallback callback)
{
    DBG("uploadFile called - fileName: " + fileName + ", folderId: " + folderId);
    
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
            
            DBG("Creating folder path: " + folderPath.joinIntoString("/"));
            DBG("File name: " + actualFileName);
            
            createFolderPath(folderPath, "root", 0, [this, actualFileName, data, callback]
                           (bool success, juce::String folderId, juce::String errorMessage)
            {
                if (success)
                {
                    DBG("Folder path created, uploading to folderId: " + folderId);
                    uploadToFolder(actualFileName, data, folderId, callback);
                }
                else
                {
                    DBG("Failed to create folder path: " + errorMessage);
                    if (callback)
                        callback(false, "Failed to create folder path: " + errorMessage);
                }
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
        if (!isTokenValid())
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, {}, "Authentication required");
            });
            return;
        }
        
        juce::URL url(endpoint);
        juce::StringPairArray headers;
        headers.set("Authorization", "Bearer " + accessToken);
        
        juce::String headerString;
        for (int i = 0; i < headers.size(); ++i)
        {
            headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                headerString += "\r\n";
        }

        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                        .withExtraHeaders(headerString)
                        .withConnectionTimeoutMs(30000);
        
        auto stream = url.createInputStream(options);
        
        auto streamPtr = stream.release();
        juce::MessageManager::callAsync([streamPtr, callback]()
        {
            std::unique_ptr<juce::InputStream> stream(streamPtr);
            if (stream != nullptr)
            {
                juce::MemoryBlock data;
                stream->readIntoMemoryBlock(data);
                
                if (callback)
                    callback(true, data, "");
            }
            else
            {
                if (callback)
                    callback(false, {}, "Failed to download file");
            }
        });
    });
}

void R2GoogleDriveProvider::deleteFile(const juce::String& fileId, FileOperationCallback callback)
{
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files/" + fileId;
    
    makeAPIRequest(endpoint, "DELETE", {}, "", [callback](bool success, const juce::String& response)
    {
        if (callback)
        {
            if (success)
                callback(true, "File deleted successfully");
            else
                callback(false, "Failed to delete file");
        }
    });
}

void R2GoogleDriveProvider::createFolder(const juce::String& folderName, const juce::String& parentId,
                                        FileOperationCallback callback)
{
    juce::String metadata = "{\n";
    metadata += "  \"name\": \"" + folderName + "\",\n";
    metadata += "  \"mimeType\": \"application/vnd.google-apps.folder\"";
    if (parentId.isNotEmpty() && parentId != "root")
    {
        metadata += ",\n  \"parents\": [\"" + parentId + "\"]";
    }
    metadata += "\n}";
    
    juce::String endpoint = "https://www.googleapis.com/drive/v3/files";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/json");
    
    makeAPIRequest(endpoint, "POST", headers, metadata, [callback](bool success, const juce::String& response)
    {
        if (!success)
        {
            if (callback)
                callback(false, "Failed to create folder - API request failed");
            return;
        }
        
        try
        {
            auto json = juce::JSON::parse(response);
            if (json.isObject())
            {
                auto* obj = json.getDynamicObject();
                if (obj->hasProperty("id"))
                {
                    // Folder created successfully
                    if (callback)
                        callback(true, "Folder created successfully");
                    return;
                }
                else
                {
                    juce::String error = "Unknown error";
                    if (obj->hasProperty("error"))
                    {
                        auto errorObj = obj->getProperty("error");
                        if (errorObj.isObject())
                        {
                            auto* errObj = errorObj.getDynamicObject();
                            if (errObj->hasProperty("message"))
                            {
                                error = errObj->getProperty("message").toString();
                            }
                        }
                    }
                    if (callback)
                        callback(false, "Failed to create folder: " + error);
                    return;
                }
            }
        }
        catch (...)
        {
            if (callback)
                callback(false, "Failed to parse folder creation response");
            return;
        }
        
        if (callback)
            callback(false, "Invalid response format");
    });
}

void R2GoogleDriveProvider::saveTokens()
{
    auto tokenFile = getTokenFile();
    
    juce::DynamicObject::Ptr tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", static_cast<juce::int64>(tokenExpiry.toMilliseconds()));
    
    auto json = juce::JSON::toString(juce::var(tokenData.get()));
    
    if (tokenFile.replaceWithText(json))
    {
        DBG("Tokens saved successfully");
    }
    else
    {
        DBG("Failed to save tokens");
    }
}

bool R2GoogleDriveProvider::loadTokens()
{
    auto tokenFile = getTokenFile();
    
    if (!tokenFile.exists())
        return false;
    
    auto json = tokenFile.loadFileAsString();
    auto tokenData = juce::JSON::parse(json);
    
    if (tokenData.isObject())
    {
        auto* obj = tokenData.getDynamicObject();
        
        if (obj->hasProperty("access_token"))
            accessToken = obj->getProperty("access_token").toString();
        
        if (obj->hasProperty("refresh_token"))
            refreshToken = obj->getProperty("refresh_token").toString();
        
        if (obj->hasProperty("token_expiry"))
        {
            auto expiryMs = static_cast<juce::int64>(obj->getProperty("token_expiry"));
            tokenExpiry = juce::Time(expiryMs);
        }
        
        if (isTokenValid())
        {
            currentStatus = Status::Authenticated;
            DBG("Tokens loaded successfully");
            return true;
        }
        else if (refreshToken.isNotEmpty())
        {
            DBG("Access token expired, but refresh token available");
            return true;
        }
    }
    
    return false;
}

juce::File R2GoogleDriveProvider::getTokenFile() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto cloudDocDir = appDataDir.getChildFile("CloudDoc");
    
    if (!cloudDocDir.exists())
    {
        cloudDocDir.createDirectory();
    }
    
    return cloudDocDir.getChildFile("google_drive_tokens.json");
}

} // namespace r2juce

