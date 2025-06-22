#include "R2OneDriveProvider.h"

namespace r2juce {

//==============================================================================
// R2OneDriveProvider::OAuth2Handler - OneDrive認証ハンドラー
//==============================================================================

R2OneDriveProvider::OAuth2Handler::OAuth2Handler(R2OneDriveProvider& parent) : provider(parent)
{
    // JUCE 8の正しいAPIを使用してWebBrowserComponentを設定
    juce::WebBrowserComponent::Options options;
    
    // ネイティブ統合を有効化（必須）
    options = options.withNativeIntegrationEnabled(true);
    
    // JavaScript関数を登録して認証コードを受信
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
    
    // URLナビゲーションイベントを監視
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
    
    // 初期化時にURL監視スクリプトを注入
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
    
    // UserAgentを設定
    options = options.withUserAgent("CloudDoc/1.0 (JUCE WebBrowser)");
    
    webBrowser = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(*webBrowser);
}

void R2OneDriveProvider::OAuth2Handler::startAuthentication(const juce::String& clientId, R2CloudStorageProvider::AuthCallback callback)
{
    authCallback = callback;
    currentClientId = clientId;
    
    // リダイレクトURIを設定
    redirectUri = "http://localhost:8080/callback";
    
    // Microsoft OAuth2認証URLを構築
    juce::String authUrl = "https://login.microsoftonline.com/consumers/oauth2/v2.0/authorize?";
    authUrl += "client_id=" + juce::URL::addEscapeChars(clientId, false);
    authUrl += "&redirect_uri=" + juce::URL::addEscapeChars(redirectUri, false);
    authUrl += "&response_type=code";
    authUrl += "&scope=" + juce::URL::addEscapeChars("Files.ReadWrite offline_access", false);
    authUrl += "&response_mode=query";
    authUrl += "&state=" + generateStateParameter();
    
    DBG("Starting OneDrive authentication with URL: " + authUrl);
    
    // 認証URLに移動
    webBrowser->goToURL(authUrl);
}

void R2OneDriveProvider::OAuth2Handler::resized()
{
    webBrowser->setBounds(getLocalBounds());
}

juce::String R2OneDriveProvider::OAuth2Handler::generateStateParameter()
{
    // CSRF攻撃防止のための状態パラメータ生成
    auto randomValue = juce::Random::getSystemRandom().nextInt64();
    stateParameter = juce::String::toHexString(static_cast<juce::int64>(randomValue));
    return stateParameter;
}

void R2OneDriveProvider::OAuth2Handler::checkAuthCallback(const juce::String& url)
{
    DBG("OneDrive Checking URL: " + url);
    
    if (url.startsWith(redirectUri))
    {
        DBG("OneDrive Redirect URI matched, extracting auth code");
        
        auto authCode = extractAuthCode(url);
        auto state = extractState(url);
        auto error = extractError(url);
        
        // 状態パラメータの検証
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
        // URLにエラーが含まれている場合
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
    
    // プロバイダーに認証コードを伝達してトークン交換を開始
    provider.exchangeAuthCodeForTokens(authCode, [this](bool success, const juce::String& errorMessage)
    {
        if (authCallback)
        {
            juce::MessageManager::callAsync([authCallback = this->authCallback, success, errorMessage]()
            {
                authCallback(success, errorMessage);
            });
        }
    });
}

void R2OneDriveProvider::OAuth2Handler::handleAuthError(const juce::String& error)
{
    DBG("OneDrive Authentication error: " + error);
    showErrorPage(error);
    
    if (authCallback)
    {
        juce::MessageManager::callAsync([authCallback = this->authCallback, error]()
        {
            authCallback(false, error);
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
    <title>認証成功</title>
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
        <h1>認証成功！</h1>
        <p>OneDriveへの接続が完了しました。</p>
        <p>このウィンドウは閉じて構いません。</p>
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
    <title>認証エラー</title>
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
        <h1>認証失敗</h1>
        <p>OneDrive認証中にエラーが発生しました。</p>
        <div class="error-detail">)" + error + R"(</div>
        <p>ウィンドウを閉じて再試行してください。</p>
    </div>
</body>
</html>
        )";
        
    webBrowser->goToURL("data:text/html;charset=utf-8," +
                       juce::URL::addEscapeChars(errorHTML, false));
}

//==============================================================================
// R2OneDriveProvider - メイン実装
//==============================================================================

R2OneDriveProvider::R2OneDriveProvider()
{
    DBG("=== R2OneDriveProvider Constructor ===");
    currentStatus = Status::NotAuthenticated;
    bool tokensLoaded = loadTokens(); // 保存されたトークンをロード
    DBG("Tokens loaded: " + juce::String(tokensLoaded ? "true" : "false"));
    DBG("Current status: " + juce::String((int)currentStatus));
}

void R2OneDriveProvider::setClientCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    this->clientId = clientId;
    this->clientSecret = clientSecret;
}

void R2OneDriveProvider::authenticate(AuthCallback callback)
{
    DBG("=== R2OneDriveProvider::authenticate() ===");
    DBG("Token valid: " + juce::String(isTokenValid() ? "true" : "false"));
    DBG("Refresh token empty: " + juce::String(refreshToken.isEmpty() ? "true" : "false"));

    if (clientId.isEmpty() || clientSecret.isEmpty())
    {
        DBG("OneDrive Client credentials not set - clientId: " + clientId + ", clientSecret: " + clientSecret);
        if (callback)
            callback(false, "Client credentials not set");
        return;
    }
    
    DBG("OneDrive Client credentials are set");
    
    // 既存の有効なトークンがある場合
    if (isTokenValid())
    {
        DBG("OneDrive Token is valid, already authenticated");
        currentStatus = Status::Authenticated;
        if (callback)
            callback(true, "Already authenticated");
        return;
    }
    
    DBG("OneDrive Token not valid, checking refresh token");
    
    // リフレッシュトークンがある場合は先にそれを試す
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
                // リフレッシュに失敗した場合、新しい認証フローを開始
                startNewAuthFlow(callback);
            }
        });
        return;
    }
    
    DBG("OneDrive No refresh token, starting new auth flow");
    
    // 新しい認証フローを開始
    startNewAuthFlow(callback);
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
    // Microsoft Graph API用のトークン交換
    juce::String postData = "grant_type=authorization_code";
    postData += "&code=" + juce::URL::addEscapeChars(authCode, false);
    postData += "&redirect_uri=" + juce::URL::addEscapeChars("http://localhost:8080/callback", false);
    postData += "&client_id=" + juce::URL::addEscapeChars(clientId, false);
    postData += "&client_secret=" + juce::URL::addEscapeChars(clientSecret, false);
    postData += "&scope=" + juce::URL::addEscapeChars("Files.ReadWrite offline_access", false);
    
    // HTTPリクエストを作成（Microsoft用エンドポイント）
    juce::URL tokenUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
    tokenUrl = tokenUrl.withPOSTData(postData);
    
    // HTTPヘッダーを設定
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/x-www-form-urlencoded");
    headers.set("Accept", "application/json");
    
    // バックグラウンドスレッドでリクエストを実行
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

void R2OneDriveProvider::setTokens(const juce::String& accessToken, const juce::String& refreshToken)
{
    this->accessToken = accessToken;
    this->refreshToken = refreshToken;
    
    // 1時間後に期限切れとして設定（安全のため）
    this->tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::hours(1);
    
    this->currentStatus = Status::Authenticated;
    saveTokens();
    
    DBG("OneDrive Tokens set successfully");
}

void R2OneDriveProvider::downloadFileWithPath(const juce::String& filePath, DownloadCallback callback)
{
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root:/" + juce::URL::addEscapeChars(filePath, false) + ":/content";
    
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
        
        if (stream != nullptr)
        {
            juce::MemoryBlock data;
            stream->readIntoMemoryBlock(data);
            
            juce::MessageManager::callAsync([callback, data]()
            {
                if (callback)
                    callback(true, data, "");
            });
        }
        else
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, {}, "Failed to download file");
            });
        }
    });
}

void R2OneDriveProvider::parseTokenResponse(const juce::String& response, std::function<void(bool, juce::String)> callback)
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
                    tokenExpiry = juce::Time::getCurrentTime() + juce::RelativeTime::seconds(expiresIn - 300); // 5分の余裕
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

void R2OneDriveProvider::signOut()
{
    accessToken.clear();
    refreshToken.clear();
    tokenExpiry = juce::Time();
    currentStatus = Status::NotAuthenticated;
    
    // 保存されたトークンファイルを削除
    auto tokenFile = getTokenFile();
    if (tokenFile.exists())
    {
        tokenFile.deleteFile();
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
    return accessToken.isNotEmpty() &&
           (tokenExpiry == juce::Time() || juce::Time::getCurrentTime() < tokenExpiry);
}

void R2OneDriveProvider::refreshAccessToken(std::function<void(bool)> callback)
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
    postData += "&scope=" + juce::URL::addEscapeChars("Files.ReadWrite offline_access", false);
    
    juce::URL tokenUrl("https://login.microsoftonline.com/common/oauth2/v2.0/token");
    tokenUrl = tokenUrl.withPOSTData(postData);
    
    juce::StringPairArray headers;
    headers.set("Content-Type", "application/x-www-form-urlencoded");
    headers.set("Accept", "application/json");
    
    juce::Thread::launch([this, tokenUrl, headers, callback]()
    {
        // StringPairArrayをHTTPヘッダー文字列に変換
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

void R2OneDriveProvider::makeAPIRequest(const juce::String& endpoint, const juce::String& method,
                                       const juce::StringPairArray& headers, const juce::String& postData,
                                       std::function<void(bool, juce::String)> callback)
{
    if (!isTokenValid())
    {
        // トークンが無効な場合はリフレッシュを試行
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
        
        // StringPairArrayをHTTPヘッダー文字列に変換
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

void R2OneDriveProvider::listFiles(const juce::String& folderId, FileListCallback callback)
{
    // Microsoft Graph API を使用してファイル一覧を取得
    juce::String endpoint = "https://graph.microsoft.com/v1.0/me/drive/root/children";
    
    // 特定のフォルダIDが指定されている場合
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
                                
                                // フォルダかファイルかの判定
                                fileInfo.isFolder = fileObj->hasProperty("folder");
                                
                                // MIMEタイプの設定
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
                                
                                // ファイルサイズ
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
                                
                                // 最終更新日時
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
    
    // Microsoft Graph API を使用してファイルをアップロード
    juce::String endpoint;
    
    if (folderId == "path")
    {
        // パス形式での保存（フォルダ込みのファイル名）
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
        
        // MemoryOutputStreamを使ってバイナリデータを準備
        juce::MemoryOutputStream bodyStream;
        bodyStream.write(data.getData(), data.getSize());
        
        // URLにPOSTデータとして設定
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
        
        // StringPairArrayをHTTPヘッダー文字列に変換
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
    // Microsoft Graph API を使用してフォルダを作成
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
    auto tokenFile = getTokenFile();
    
    juce::DynamicObject::Ptr tokenData = new juce::DynamicObject();
    tokenData->setProperty("access_token", accessToken);
    tokenData->setProperty("refresh_token", refreshToken);
    tokenData->setProperty("token_expiry", static_cast<juce::int64>(tokenExpiry.toMilliseconds()));
    
    auto json = juce::JSON::toString(juce::var(tokenData.get()));
    
    if (tokenFile.replaceWithText(json))
    {
        DBG("OneDrive Tokens saved successfully");
    }
    else
    {
        DBG("OneDrive Failed to save tokens");
    }
}

bool R2OneDriveProvider::loadTokens()
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
        
        // トークンが有効な場合は認証済み状態に設定
        if (isTokenValid())
        {
            currentStatus = Status::Authenticated;
            DBG("OneDrive Tokens loaded successfully");
            return true;
        }
        else if (refreshToken.isNotEmpty())
        {
            DBG("OneDrive Access token expired, but refresh token available");
            return true;
        }
    }
    
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
