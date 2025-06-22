#include "R2OneDriveProvider.h"

namespace r2juce {

R2OneDriveProvider::OAuth2Handler::OAuth2Handler(R2OneDriveProvider& parent) : provider(parent)
{
    juce::WebBrowserComponent::Options options;
    
    options = options.withNativeIntegrationEnabled(true);
    
    options = options.withNativeFunction("authComplete",
        [this](const juce::Array<juce::var>& args, juce::WebBrowserComponent::NativeFunctionCompletion completion)
        {
            DBG("OneDrive authComplete called with " + juce::String(args.size()) + " args");
            if (args.size() > 0)
            {
                auto authCode = args[0].toString();
                if (authCode.isNotEmpty())
                {
                    DBG("OneDrive Auth code received: " + authCode);
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
            DBG("OneDrive Auth error: " + error);
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
                    DBG("OneDrive URL changed to: " + url);
                    checkAuthCallback(url);
                }
            }
        });
    
    juce::String monitoringScript = R"(
        // Script to monitor URL changes for OneDrive auth
        let lastUrl = window.location.href;
        
        function checkUrlChange() {
            const currentUrl = window.location.href;
            if (currentUrl !== lastUrl) {
                lastUrl = currentUrl;
                
                // Notify JUCE backend of URL change
                if (window.__JUCE__ && window.__JUCE__.backend) {
                    window.__JUCE__.backend.emitEvent('urlChanged', { url: currentUrl });
                }
                
                if (currentUrl.includes('login.microsoftonline.com/common/oauth2/nativeclient')) {
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

void R2OneDriveProvider::OAuth2Handler::startAuthentication(const juce::String& clientId, R2CloudStorageProvider::AuthCallback callback)
{
   authCallback = callback;
   currentClientId = clientId;
   
   redirectUri = "https://login.microsoftonline.com/common/oauth2/nativeclient";
   
   juce::String authUrl = "https://login.microsoftonline.com/consumers/oauth2/v2.0/authorize?";
   authUrl += "client_id=" + juce::URL::addEscapeChars(clientId, false);
   authUrl += "&redirect_uri=" + juce::URL::addEscapeChars(redirectUri, false);
   authUrl += "&response_type=code";
   authUrl += "&scope=" + juce::URL::addEscapeChars("Files.ReadWrite offline_access", false);
   authUrl += "&response_mode=query";
   authUrl += "&state=" + generateStateParameter();
   
   DBG("Starting OneDrive authentication with URL: " + authUrl);
   DBG("Platform: " + juce::SystemStats::getOperatingSystemName());
   
   webBrowser->goToURL(authUrl);
}

void R2OneDriveProvider::OAuth2Handler::resized()
{
    webBrowser->setBounds(getLocalBounds());
}

juce::String R2OneDriveProvider::OAuth2Handler::generateStateParameter()
{
    auto randomValue = juce::Random::getSystemRandom().nextInt64();
    stateParameter = juce::String::toHexString(static_cast<juce::int64>(randomValue));
    return stateParameter;
}

void R2OneDriveProvider::OAuth2Handler::checkAuthCallback(const juce::String& url)
{
    DBG("OneDrive Checking URL: " + url);
    
    if (url.startsWith("https://login.microsoftonline.com/common/oauth2/nativeclient"))
    {
        DBG("OneDrive nativeclient redirect URI matched, extracting auth code");
        
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

void R2OneDriveProvider::OAuth2Handler::handleAuthSuccess(const juce::String& authCode)
{
    DBG("OneDrive Authentication successful");
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

void R2OneDriveProvider::OAuth2Handler::handleAuthError(const juce::String& error)
{
    DBG("OneDrive Authentication error: " + error);
    showErrorPage(error);

    auto currentCallback = authCallback;

    if (currentCallback)
    {
        juce::MessageManager::callAsync([currentCallback, error]()
            {
                currentCallback(false, error);
            });
    }
}

juce::String R2OneDriveProvider::OAuth2Handler::extractURLParameter(const juce::String& url, const juce::String& paramName)
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

juce::String R2OneDriveProvider::OAuth2Handler::extractAuthCode(const juce::String& url)
{
    return extractURLParameter(url, "code");
}

juce::String R2OneDriveProvider::OAuth2Handler::extractState(const juce::String& url)
{
    return extractURLParameter(url, "state");
}

juce::String R2OneDriveProvider::OAuth2Handler::extractError(const juce::String& url)
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

void R2OneDriveProvider::OAuth2Handler::showSuccessPage()
{
    juce::String successHTML = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Authentication Success</title>
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            text-align: center; 
            padding: 50px; 
            background: linear-gradient(135deg, #0078d4, #106ebe); 
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
        <h1>Authentication Success!</h1>
        <p>OneDrive connection completed.</p>
        <p>You can close this window.</p>
    </div>
</body>
</html>
        )";
        
    webBrowser->goToURL("data:text/html;charset=utf-8," +
                       juce::URL::addEscapeChars(successHTML, false));
}

void R2OneDriveProvider::OAuth2Handler::showErrorPage(const juce::String& error)
{
    juce::String errorHTML = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>Authentication Error</title>
    <style>
        body { 
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif;
            text-align: center; 
            padding: 50px; 
            background: linear-gradient(135deg, #d13438, #b52d31); 
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
        <h1>Authentication Failed</h1>
        <p>An error occurred during OneDrive authentication.</p>
        <div class="error-detail">)" + error + R"(</div>
        <p>Please close this window and try again.</p>
    </div>
</body>
</html>
        )";
        
    webBrowser->goToURL("data:text/html;charset=utf-8," +
                       juce::URL::addEscapeChars(errorHTML, false));
}

R2OneDriveProvider::R2OneDriveProvider()
{
    DBG("=== R2OneDriveProvider Constructor ===");
    currentStatus = Status::NotAuthenticated;
    bool tokensLoaded = loadTokens();
    DBG("Tokens loaded from file: " + juce::String(tokensLoaded ? "true" : "false"));
    if (tokensLoaded)
    {
        DBG("Current status after loading: " + juce::String((int)currentStatus));
    }
}

void R2OneDriveProvider::setClientCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    this->clientId = clientId;
    this->clientSecret = clientSecret;
    DBG("OneDrive credentials set - clientId: " + clientId + ", clientSecret: " + (clientSecret.isEmpty() ? "empty" : "provided"));
}

void R2OneDriveProvider::authenticate(AuthCallback callback)
{
    DBG("=== R2OneDriveProvider::authenticate() ===");
    DBG("Current status at start: " + juce::String((int)currentStatus));
    DBG("Token valid: " + juce::String(isTokenValid() ? "true" : "false"));
    DBG("Refresh token empty: " + juce::String(refreshToken.isEmpty() ? "true" : "false"));
    DBG("Access token length: " + juce::String(accessToken.length()));
    DBG("Refresh token length: " + juce::String(refreshToken.length()));

    if (clientId.isEmpty() || clientSecret.isEmpty())
    {
        DBG("OneDrive Client credentials not set - clientId: " + clientId + ", clientSecret: " + clientSecret);
        if (callback)
            callback(false, "Client credentials not set");
        return;
    }
    
    DBG("OneDrive Client credentials are set");
    
    // CRITICAL: Check current status first!
    if (currentStatus == Status::Authenticated && isTokenValid())
    {
        DBG("OneDrive ALREADY AUTHENTICATED with valid token!");
        if (callback)
            callback(true, "Already authenticated");
        return;
    }
    
    if (isTokenValid())
    {
        DBG("OneDrive Token is valid, setting authenticated status");
        currentStatus = Status::Authenticated;
        if (callback)
            callback(true, "Already authenticated");
        return;
    }
    
    DBG("OneDrive Token not valid, checking refresh token");
    
    if (refreshToken.isNotEmpty())
    {
        DBG("OneDrive Refresh token available, trying to refresh");
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
    
    DBG("OneDrive No refresh token, device flow authentication required");
    
    currentStatus = Status::NotAuthenticated;
    if (callback)
        callback(false, "device flow authentication required");
}

void R2OneDriveProvider::startNewAuthFlow(AuthCallback callback)
{
    currentStatus = Status::Authenticating;
    
    if (!oauth2Handler)
    {
        oauth2Handler = std::make_unique<OAuth2Handler>(*this);
    }
    
    oauth2Handler->startAuthentication(clientId, callback);
}

void R2OneDriveProvider::exchangeAuthCodeForTokens(const juce::String& authCode, std::function<void(bool, juce::String)> callback)
{
    DBG("=== OneDrive exchangeAuthCodeForTokens ===");
    DBG("Auth code: " + authCode.substring(0, 20) + "...");
    
    juce::String postData = "grant_type=authorization_code";
    postData += "&code=" + juce::URL::addEscapeChars(authCode, false);
    postData += "&redirect_uri=" + juce::URL::addEscapeChars("https://login.microsoftonline.com/common/oauth2/nativeclient", false);
    postData += "&client_id=" + juce::URL::addEscapeChars(clientId, false);
    postData += "&client_secret=" + juce::URL::addEscapeChars(clientSecret, false);
    
    juce::URL tokenUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
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
            DBG("Token exchange response: " + response);
            
            juce::MessageManager::callAsync([this, response, callback]()
            {
                parseTokenResponse(response, callback);
            });
        }
        else
        {
            DBG("Failed to create HTTP request for token exchange");
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, "Failed to create HTTP request");
            });
        }
    });
}

void R2OneDriveProvider::setTokens(const juce::String& accessToken, const juce::String& refreshToken)
{
    DBG("=== OneDrive setTokens ===");
    DBG("Access token preview: " + accessToken.substring(0, 20) + "...");
    DBG("Refresh token preview: " + refreshToken.substring(0, 20) + "...");
    
    this->accessToken = accessToken;
    this->refreshToken = refreshToken;
    
    this->tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::hours(1) - juce::RelativeTime::seconds(300);
    
    this->currentStatus = Status::Authenticated;
    
    saveTokens();
    
    DBG("OneDrive Tokens set successfully");
}

void R2OneDriveProvider::downloadFileWithPath(const juce::String& filePath, DownloadCallback callback)
{
    DBG("=== OneDrive downloadFileWithPath DEBUG ===");
    DBG("File path: " + filePath);
    DBG("Access token available: " + juce::String(accessToken.isNotEmpty() ? "true" : "false"));
    
    if (accessToken.isEmpty())
    {
        DBG("ERROR: No access token available for download.");
        juce::MessageManager::callAsync([callback]()
        {
            if (callback)
                callback(false, {}, "Authentication required: No access token.");
        });
        return;
    }

    DBG("Token first 30 chars: " + accessToken.substring(0, juce::jmin (30, accessToken.length())) + "...");
    DBG("Token length: " + juce::String(accessToken.length()));

    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(filePath, false) + ":/content";
    DBG("Endpoint: " + endpoint);

    auto performDownloadRequest = [this, endpoint, callback, filePath](bool initialCall)
    {
        DBG("=== OneDrive Token Validation (before download request) ===");
        DBG("Has access token: " + juce::String(accessToken.isNotEmpty() ? "true" : "false"));
        DBG("Token not expired: " + juce::String(tokenExpiry == juce::Time() || tokenExpiry > juce::Time::getCurrentTime() ? "true" : "false"));
        DBG("Token preview: " + accessToken.substring(0, juce::jmin (30, accessToken.length())) + "...");
        DBG("Token expiry: " + tokenExpiry.toString(true, true, true, true));
        DBG("Current time: " + juce::Time::getCurrentTime().toString(true, true, true, true));
        DBG("Token valid: " + juce::String(isTokenValid() ? "true" : "false"));

        if (!isTokenValid())
        {
            DBG("Token validation failed before download request. Attempting refresh if not a retry.");
            if (initialCall)
            {
                juce::MessageManager::callAsync([this, filePath, callback]()
                {
                    refreshAccessToken([this, filePath, callback](bool refreshSuccess)
                    {
                        if (refreshSuccess)
                        {
                            DBG("Token refreshed successfully, retrying download.");
                            downloadFileWithPath(filePath, callback);
                        }
                        else
                        {
                            DBG("Failed to refresh token, download failed.");
                            if (callback)
                                callback(false, {}, "Authentication required: Token refresh failed.");
                        }
                    });
                });
            }
            else
            {
                 juce::MessageManager::callAsync([callback]()
                    {
                        if (callback)
                            callback(false, {}, "Authentication required: Token invalid after refresh attempt.");
                    });
            }
            return;
        }

        juce::URL url(endpoint);
        juce::StringPairArray headers;
        headers.set("Authorization", "Bearer " + accessToken);
        headers.set("User-Agent", "CloudDoc/1.0");
        headers.set("Accept", "*/*");
        
        headers.set("Accept-Encoding", "");

        #if JUCE_WINDOWS
        headers.set("Cache-Control", "no-cache");
        headers.set("Connection", "keep-alive");
        #endif

        DBG("=== Headers being sent for download ===");
        juce::String headerDebugString;
        for (int i = 0; i < headers.size(); ++i)
        {
            juce::String headerName = headers.getAllKeys()[i];
            juce::String headerValue = headers.getAllValues()[i];

            if (headerName == "Authorization")
            {
                headerDebugString += headerName + ": Bearer " + headerValue.substring(7, juce::jmin (37, headerValue.length())) + "...\n";
            }
            else
            {
                headerDebugString += headerName + ": " + headerValue + "\n";
            }
        }
        DBG(headerDebugString.trim());

        juce::String finalHeaderString;
        for (int i = 0; i < headers.size(); ++i)
        {
            finalHeaderString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
            if (i < headers.size() - 1)
                finalHeaderString += "\r\n";
        }

        DBG("=== Final header string for download ===");
        DBG("Header string length: " + juce::String(finalHeaderString.length()));

        #if JUCE_WINDOWS
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders(finalHeaderString)
                           .withConnectionTimeoutMs(30000)
                           .withNumRedirectsToFollow(10)
                           .withHttpRequestCmd("GET");
        #else
        auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                           .withExtraHeaders(finalHeaderString)
                           .withConnectionTimeoutMs(30000)
                           .withNumRedirectsToFollow(10);
        #endif

        DBG("Creating input stream for download...");
        juce::Thread::launch([this, url, options, callback, initialCall, filePath]() mutable {
            std::unique_ptr<juce::InputStream> baseStream;
            juce::WebInputStream* webStream = nullptr;
            
            try {
                baseStream = url.createInputStream(options);
                webStream = dynamic_cast<juce::WebInputStream*>(baseStream.get());

                if (webStream == nullptr)
                {
                    DBG("ERROR: Stream is not a WebInputStream. Cannot get HTTP status code.");
                    juce::MessageManager::callAsync([callback]() {
                        if (callback)
                            callback(false, {}, "Failed to get HTTP stream type.");
                    });
                    return;
                }
                
            } catch (const std::exception& e) {
                DBG("GENERAL EXCEPTION during stream creation: " + juce::String(e.what()));
                juce::MessageManager::callAsync([callback, errorMsg = juce::String(e.what())]() {
                    if (callback)
                        callback(false, {}, "General error creating stream: " + errorMsg);
                });
                return;
            }

            if (baseStream != nullptr && webStream != nullptr)
            {
                DBG("Stream created successfully for download.");
                int statusCode = webStream->getStatusCode();
                DBG("HTTP Status Code: " + juce::String(statusCode));

                juce::MemoryBlock data;
                size_t bytesRead = baseStream->readIntoMemoryBlock(data);

                DBG("Data read, size: " + juce::String(static_cast<int>(data.getSize())) + " (bytesRead: " + juce::String(static_cast<int>(bytesRead)) + ")");

                juce::String responseContent = juce::String::createStringFromData(data.getData(), static_cast<int>(data.getSize()));
                DBG("Full response from download: " + responseContent);

                if (statusCode == 401 || (responseContent.contains("error") && responseContent.contains("unauthenticated")))
                {
                    DBG("ERROR: Received unauthenticated (401) response for download! Attempting token refresh if not already attempted.");
                    
                    if (initialCall)
                    {
                        juce::MessageManager::callAsync([this, filePath, callback]()
                        {
                            refreshAccessToken([this, filePath, callback](bool refreshSuccess)
                            {
                                if (refreshSuccess)
                                {
                                    DBG("Token refreshed successfully, retrying download.");
                                    downloadFileWithPath(filePath, callback);
                                }
                                else
                                {
                                    DBG("Failed to refresh token, download failed.");
                                    if (callback)
                                        callback(false, {}, "Authentication required: Token refresh failed.");
                                }
                            });
                        });
                    }
                    else
                    {
                         juce::MessageManager::callAsync([callback, responseContent]()
                            {
                                if (callback)
                                    callback(false, {}, "Unauthenticated after retry: " + responseContent);
                            });
                    }
                    return;
                }
                else if (statusCode >= 400)
                {
                    juce::MessageManager::callAsync([callback, responseContent, statusCode]()
                        {
                            if (callback)
                                callback(false, {}, "API Error (Status " + juce::String(statusCode) + "): " + responseContent);
                        });
                    return;
                }
                
                juce::MessageManager::callAsync([callback, data]()
                    {
                        if (callback)
                            callback(true, data, "");
                    });
            }
            else
            {
                DBG("Failed to create input stream for download (stream == nullptr).");
                juce::MessageManager::callAsync([callback]()
                    {
                        if (callback)
                            callback(false, {}, "Failed to create input stream for download (nullptr).");
                    });
            }
        });
    };

    juce::MessageManager::callAsync([performDownloadRequest]() {
        performDownloadRequest(true);
    });
}

void R2OneDriveProvider::cancelAuthentication()
{
    DBG("R2OneDriveProvider::cancelAuthentication() called");
    
    if (oauth2Handler)
    {
        oauth2Handler.reset();
    }
    
    currentStatus = Status::NotAuthenticated;
}

void R2OneDriveProvider::parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> callback)
{
    DBG("=== OneDrive parseTokenResponse ===");
    DBG("Response: " + response);
    
    try
    {
        auto json = juce::JSON::parse(response);
        
        if (json.isObject())
        {
            auto* obj = json.getDynamicObject();
            
            if (obj->hasProperty("access_token"))
            {
                juce::String newAccessToken = obj->getProperty("access_token").toString();
                juce::String newRefreshToken;
                
                if (obj->hasProperty("refresh_token"))
                {
                    newRefreshToken = obj->getProperty("refresh_token").toString();
                }
                else
                {
                    // Keep existing refresh token during token refresh
                    newRefreshToken = refreshToken;
                }
                
                juce::Time newTokenExpiry;
                if (obj->hasProperty("expires_in"))
                {
                    auto expiresIn = static_cast<int>(obj->getProperty("expires_in"));
                    newTokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(expiresIn - 300);
                }
                else
                {
                    newTokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::hours(1) - juce::RelativeTime::seconds(300);
                }
                
                accessToken = newAccessToken;
                refreshToken = newRefreshToken;
                tokenExpiry = newTokenExpiry;
                
                currentStatus = Status::Authenticated;
                
                saveTokens();
                DBG("Tokens saved");
                
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
                    
                DBG("Token response error: " + errorMessage);
                currentStatus = Status::Error;
                if (callback)
                    callback(false, errorMessage);
                return;
            }
        }
        
        DBG("Invalid token response format");
        currentStatus = Status::Error;
        if (callback)
            callback(false, "Invalid token response");
    }
    catch (...)
    {
        DBG("Exception parsing token response");
        currentStatus = Status::Error;
        if (callback)
            callback(false, "Failed to parse token response");
    }
}

void R2OneDriveProvider::signOut()
{
    DBG("OneDrive signOut called");
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = juce::Time();
    currentStatus = Status::NotAuthenticated;
    
    auto tokenFile = getTokenFile();
    if (tokenFile.exists())
    {
        bool deleted = tokenFile.deleteFile();
        DBG("Token file deleted: " + juce::String(deleted ? "true" : "false"));
    }
}

R2CloudStorageProvider::Status R2OneDriveProvider::getAuthStatus() const
{
    return currentStatus;
}

juce::String R2OneDriveProvider::getDisplayName() const
{
    return "OneDrive";
}

bool R2OneDriveProvider::isTokenValid() const
{
    bool hasToken = accessToken.isNotEmpty();
    bool notExpired = (tokenExpiry == juce::Time() || juce::Time::getCurrentTime() < tokenExpiry);
    
    DBG("=== OneDrive Token Validation ===");
    DBG("Has access token: " + juce::String(hasToken ? "true" : "false"));
    DBG("Token not expired: " + juce::String(notExpired ? "true" : "false"));
    if (hasToken)
    {
        DBG("Token preview: " + accessToken.substring(0, 20) + "...");
    }
    if (tokenExpiry != juce::Time())
    {
        DBG("Token expiry: " + tokenExpiry.toString(true, true));
        DBG("Current time: " + juce::Time::getCurrentTime().toString(true, true));
    }
    
    return hasToken && notExpired;
}

void R2OneDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
{
    DBG("=== OneDrive Refresh Token ===");
    DBG("Refresh token available: " + juce::String(refreshToken.isNotEmpty() ? "true" : "false"));
    
    if (refreshToken.isEmpty())
    {
        DBG("No refresh token available");
        if (callback)
            callback(false);
        return;
    }
    
    juce::String postData = "grant_type=refresh_token";
    postData += "&refresh_token=" + juce::URL::addEscapeChars(refreshToken, false);
    postData += "&client_id=" + juce::URL::addEscapeChars(clientId, false);
    postData += "&client_secret=" + juce::URL::addEscapeChars(clientSecret, false);

    juce::URL tokenUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
    tokenUrl = tokenUrl.withPOSTData(postData);

    juce::StringPairArray headers;
    headers.set("Content-Type", "application/x-www-form-urlencoded");
    headers.set("Accept", "application/json");
    headers.set("User-Agent", "CloudDoc/1.0");
    
    juce::Thread::launch([this, tokenUrl, headers, callback]()
    {
        try
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
                
                DBG("=== Refresh Token Response ===");
                DBG("Response: " + response);
                
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
                DBG("Failed to connect for token refresh");
                juce::MessageManager::callAsync([callback]()
                {
                    if (callback)
                        callback(false);
                });
            }
        }
        catch (const std::exception& e)
        {
            DBG("Exception in refreshAccessToken: " + juce::String(e.what()));
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false);
            });
        }
        catch (...)
        {
            DBG("Unknown exception in refreshAccessToken");
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false);
            });
        }
    });
}

void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method,
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
        requestHeaders.set("User-Agent", "CloudDoc/1.0");
        
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
        juce::MessageManager::callAsync([streamPtr, callback, endpoint]()
        {
            std::unique_ptr<juce::InputStream> stream(streamPtr);
            if (stream != nullptr)
            {
                auto response = stream->readEntireStreamAsString();
                DBG("=== OneDrive API Response ===");
                DBG("Endpoint: " + endpoint);
                DBG("Response: " + response);
                if (callback)
                    callback(true, response);
            }
            else
            {
                DBG("=== OneDrive API Request Failed ===");
                if (callback)
                    callback(false, "Request failed");
            }
        });
    });
}

void R2OneDriveProvider::listFiles(const juce::String& folderId, FileListCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root/children";
    
    if (folderId.isNotEmpty() && folderId != "root")
    {
        endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + folderId + "/children";
    }
    
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
                if (obj->hasProperty("value"))
                {
                    auto filesArray = obj->getProperty("value");
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
                                
                                fileInfo.isFolder = fileObj->hasProperty("folder");
                                
                                if (fileInfo.isFolder)
                                {
                                    fileInfo.mimeType = "application/vnd.microsoft.onedrive.folder";
                                }
                                else if (fileObj->hasProperty("file"))
                                {
                                    auto fileProps = fileObj->getProperty("file");
                                    if (fileProps.isObject())
                                    {
                                        auto* filePropObj = fileProps.getDynamicObject();
                                        if (filePropObj->hasProperty("mimeType"))
                                        {
                                            fileInfo.mimeType = filePropObj->getProperty("mimeType").toString();
                                        }
                                        else
                                        {
                                            fileInfo.mimeType = "application/octet-stream";
                                        }
                                    }
                                    else
                                    {
                                        fileInfo.mimeType = "application/octet-stream";
                                    }
                                }
                                
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
                                
                                if (fileObj->hasProperty("lastModifiedDateTime"))
                                {
                                    auto timeStr = fileObj->getProperty("lastModifiedDateTime").toString();
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

void R2OneDriveProvider::uploadFile(const juce::String& fileName, const juce::MemoryBlock& data,
                                   const juce::String& folderId, FileOperationCallback callback)
{
    DBG("=== OneDrive Upload File ===");
    DBG("Filename: " + fileName);
    DBG("Folder ID: " + folderId);
    DBG("Data size: " + juce::String(data.getSize()));
    
    juce::String endpoint;
    
    if (folderId == "path")
    {
        endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(fileName, false) + ":/content";
    }
    else if (folderId.isEmpty() || folderId == "root")
    {
        endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(fileName, false) + ":/content";
    }
    else
    {
        endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + folderId + ":/" + juce::URL::addEscapeChars(fileName, false) + ":/content";
    }
    
    DBG("Upload endpoint: " + endpoint);
    
    juce::Thread::launch([this, endpoint, data, callback]()
    {
        if (!isTokenValid())
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, "Authentication required");
            });
            return;
        }
        
        juce::MemoryOutputStream bodyStream;
        bodyStream.write(data.getData(), data.getSize());
        
        juce::URL url(endpoint);
        url = url.withPOSTData(bodyStream.toString());
        
        juce::StringPairArray headers;
        headers.set("Authorization", "Bearer " + accessToken);
        headers.set("Content-Type", "application/octet-stream");
        
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
                        .withHttpRequestCmd("PUT");
        
        auto stream = url.createInputStream(options);
        
        if (stream != nullptr)
        {
            auto response = stream->readEntireStreamAsString();
            
            juce::MessageManager::callAsync([callback, response]()
            {
                if (callback)
                    callback(true, "File uploaded successfully");
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, "Failed to connect to OneDrive");
            });
        }
    });
}

void R2OneDriveProvider::downloadFile(const juce::String& fileId, DownloadCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + fileId + "/content";
    
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

void R2OneDriveProvider::deleteFile(const juce::String& fileId, FileOperationCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + fileId;
    
    makeAPIRequest(endpoint, "DELETE", {}, "", [callback](bool success, const juce::String& response)
    {
        if (callback)
        {
            if (success)
                callback(true, "File deleted successfully");
            else
                callback(false, "Failed to delete file: " + response);
        }
    });
}

void R2OneDriveProvider::createFolder(const juce::String& folderName, const juce::String& parentId,
                                     FileOperationCallback callback)
{
    juce::String endpoint;
    
    if (parentId.isEmpty() || parentId == "root")
    {
        endpoint = "https://graph.microsoft.com/v1.0/me/drive/root/children";
    }
    else
    {
        endpoint = "https://graph.microsoft.com/v1.0/me/drive/items/" + parentId + "/children";
    }
    
    juce::String metadata = "{\n";
    metadata += "  \"name\": \"" + folderName + "\",\n";
    metadata += "  \"folder\": {},\n";
    metadata += "  \"@microsoft.graph.conflictBehavior\": \"rename\"\n";
    metadata += "}";
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/json");
    
    makeAPIRequest(endpoint, "POST", headers, metadata, [callback](bool success, const juce::String& response)
    {
        if (callback)
        {
            if (success)
                callback(true, "Folder created successfully");
            else
                callback(false, "Failed to create folder: " + response);
        }
    });
}

void R2OneDriveProvider::saveTokens()
{
    DBG("=== OneDrive saveTokens ===");
    
    auto tokenFile = getTokenFile();
    DBG("Token file path: " + tokenFile.getFullPathName());
    
    juce::DynamicObject::Ptr tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", static_cast<juce::int64>(tokenExpiry.toMilliseconds()));
    
    auto json = juce::JSON::toString(juce::var(tokenData.get()));
    DBG("Saving token data: " + json);
    
    auto parentDir = tokenFile.getParentDirectory();
    if (!parentDir.exists())
    {
        bool dirCreated = parentDir.createDirectory();
        DBG("Parent directory created: " + juce::String(dirCreated ? "true" : "false"));
    }
    
    bool saved = tokenFile.replaceWithText(json);
    
    if (saved)
    {
        DBG("OneDrive Tokens saved successfully");
        if (tokenFile.exists())
        {
            auto savedContent = tokenFile.loadFileAsString();
            DBG("Saved content verification: " + savedContent.substring(0, 100) + "...");
        }
    }
    else
    {
        DBG("OneDrive Failed to save tokens");
    }
}

bool R2OneDriveProvider::loadTokens()
{
    DBG("=== OneDrive loadTokens ===");
    
    auto tokenFile = getTokenFile();
    DBG("Token file path: " + tokenFile.getFullPathName());
    
    if (!tokenFile.exists())
    {
        DBG("Token file does not exist");
        return false;
    }
    
    auto json = tokenFile.loadFileAsString();
    DBG("Loaded token data: " + json);
    
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
        
        DBG("Loaded access token preview: " + accessToken.substring(0, 20) + "...");
        DBG("Loaded refresh token preview: " + refreshToken.substring(0, 20) + "...");
        DBG("Token expiry: " + tokenExpiry.toString(true, true));
        
        // Remove the old v1.0 token check - this was causing tokens to be deleted!
        // OneDrive v2.0 tokens can start with "EwB" which is valid for Microsoft Graph API
        
        if (isTokenValid())
        {
            currentStatus = Status::Authenticated;
            DBG("OneDrive Tokens loaded successfully - status set to Authenticated");
            return true;
        }
        else if (refreshToken.isNotEmpty())
        {
            DBG("OneDrive Access token expired, but refresh token available");
            currentStatus = Status::Authenticated;
            return true;
        }
    }
    
    DBG("Failed to parse token data");
    return false;
}

juce::File R2OneDriveProvider::getTokenFile() const
{
    auto appDataDir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory);
    auto cloudDocDir = appDataDir.getChildFile("CloudDoc");
    
    if (!cloudDocDir.exists())
    {
        cloudDocDir.createDirectory();
    }
    
    return cloudDocDir.getChildFile("onedrive_tokens.json");
}

} // namespace r2juce
