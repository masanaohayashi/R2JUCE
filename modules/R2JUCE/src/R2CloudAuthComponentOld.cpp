#include "R2CloudAuthComponent.h"

namespace r2juce {

R2CloudAuthComponent::R2CloudAuthComponent()
{
    setupUI();
    setWantsKeyboardFocus(true);
}

R2CloudAuthComponent::~R2CloudAuthComponent()
{
    stopAuthentication();
}

void R2CloudAuthComponent::setupUI()
{
    // タイトル
    titleLabel = std::make_unique<juce::Label>("title", TRANS("Cloud Authentication"));
    addAndMakeVisible(*titleLabel);
    titleLabel->setFont(juce::Font(juce::FontOptions(24.0f, juce::Font::bold)));
    titleLabel->setJustificationType(juce::Justification::centred);
    titleLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    
    // 指示ラベル
    instructionLabel = std::make_unique<juce::Label>("instruction", "");
    addAndMakeVisible(*instructionLabel);
    instructionLabel->setFont(juce::Font(juce::FontOptions(14.0f)));
    instructionLabel->setJustificationType(juce::Justification::centred);
    instructionLabel->setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    
    // コード表示ラベル
    codeDisplayLabel = std::make_unique<juce::Label>("code", "");
    addAndMakeVisible(*codeDisplayLabel);
    codeDisplayLabel->setFont(juce::Font(juce::FontOptions(20.0f, juce::Font::bold)));
    codeDisplayLabel->setJustificationType(juce::Justification::centred);
    codeDisplayLabel->setColour(juce::Label::textColourId, juce::Colours::yellow);
    codeDisplayLabel->setColour(juce::Label::backgroundColourId, juce::Colours::darkgrey);
    
    // URL表示ラベル（より目立つように）
    urlDisplayLabel = std::make_unique<juce::Label>("url", "");
    addAndMakeVisible(*urlDisplayLabel);
    urlDisplayLabel->setFont(juce::Font(juce::FontOptions(13.0f, juce::Font::bold)));
    urlDisplayLabel->setJustificationType(juce::Justification::centred);
    urlDisplayLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    urlDisplayLabel->setColour(juce::Label::backgroundColourId, juce::Colours::darkblue);
    urlDisplayLabel->setColour(juce::Label::outlineColourId, juce::Colours::lightblue);
    
    // ボタン類
    openUrlButton = std::make_unique<juce::TextButton>(TRANS("Open URL"));
    addAndMakeVisible(*openUrlButton);
    openUrlButton->addListener(this);
    
    copyCodeButton = std::make_unique<juce::TextButton>(TRANS("Copy Code"));
    addAndMakeVisible(*copyCodeButton);
    copyCodeButton->addListener(this);
    
    copyUrlButton = std::make_unique<juce::TextButton>(TRANS("Copy URL"));
    addAndMakeVisible(*copyUrlButton);
    copyUrlButton->addListener(this);
    
    cancelButton = std::make_unique<juce::TextButton>(TRANS("Cancel"));
    addAndMakeVisible(*cancelButton);
    cancelButton->addListener(this);
    
    // ステータスラベル
    statusLabel = std::make_unique<juce::Label>("status", TRANS("Initializing..."));
    addAndMakeVisible(*statusLabel);
    statusLabel->setFont(juce::Font(juce::FontOptions(12.0f)));
    statusLabel->setJustificationType(juce::Justification::centred);
    statusLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    
    // プログレスバー
    progressBar = std::make_unique<juce::ProgressBar>(progressValue);
    addAndMakeVisible(*progressBar);
    
    // 初期状態では非表示にするが、URL/コード表示ラベルは準備しておく
    openUrlButton->setEnabled(false);
    copyCodeButton->setEnabled(false);
    copyUrlButton->setEnabled(false);
    
    codeDisplayLabel->setText(TRANS("Waiting for code..."), juce::dontSendNotification);
    urlDisplayLabel->setText(TRANS("Waiting for URL..."), juce::dontSendNotification);
    
    setVisible(false);
}

void R2CloudAuthComponent::setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret)
{
    googleClientId = clientId;
    googleClientSecret = clientSecret;
}

void R2CloudAuthComponent::setServiceType(ServiceType type)
{
    serviceType = type;
    
    switch (type)
    {
        case ServiceType::GoogleDrive:
            titleLabel->setText(TRANS("Google Drive Authentication"), juce::dontSendNotification);
            break;
        // 今後追加予定
        // case ServiceType::Dropbox:
        //     titleLabel->setText(TRANS("Dropbox Authentication"), juce::dontSendNotification);
        //     break;
    }
}

void R2CloudAuthComponent::startAuthentication()
{
    if (deviceFlowActive)
        return;
    
    setVisible(true);
    toFront(true);
    grabKeyboardFocus();
    
    updateStatus(TRANS("Requesting authentication code..."));
    progressValue = 0.1;
    progressBar->repaint();
    
    deviceFlowActive = true;
    pollCount = 0;
    interval = 5;
    
    requestDeviceCode();
}

void R2CloudAuthComponent::stopAuthentication()
{
    deviceFlowActive = false;
    stopTimer();
    
    deviceCode.clear();
    userCode.clear();
    verificationUrl.clear();
    pollCount = 0;
    
    setVisible(false);
}

void R2CloudAuthComponent::requestDeviceCode()
{
    if (serviceType != ServiceType::GoogleDrive)
    {
        showError(TRANS("Unsupported service type"));
        return;
    }
    
    juce::String postData = "client_id=" + juce::URL::addEscapeChars(googleClientId, false);
    postData += "&scope=" + juce::URL::addEscapeChars("https://www.googleapis.com/auth/drive.file", false);
    
    makeHttpRequest("https://oauth2.googleapis.com/device/code", postData,
        [this](bool success, juce::String response, int statusCode)
        {
            juce::MessageManager::callAsync([this, success, response, statusCode]()
            {
                if (success && statusCode == 200)
                {
                    parseDeviceCodeResponse(response);
                }
                else
                {
                    showError(TRANS("Failed to request device code: ") + response);
                }
            });
        });
}

void R2CloudAuthComponent::parseDeviceCodeResponse(const juce::String& response)
{
    try
    {
        auto json = juce::JSON::parse(response);
        if (!json.isObject())
        {
            showError(TRANS("Invalid response format"));
            return;
        }
        
        auto* obj = json.getDynamicObject();
        
        if (obj->hasProperty("error"))
        {
            auto error = obj->getProperty("error").toString();
            auto errorDesc = obj->getProperty("error_description").toString();
            showError(error + (errorDesc.isNotEmpty() ? ": " + errorDesc : ""));
            return;
        }
        
        deviceCode = obj->getProperty("device_code").toString();
        userCode = obj->getProperty("user_code").toString();
        verificationUrl = obj->getProperty("verification_url").toString();
        
        if (obj->hasProperty("interval"))
            interval = static_cast<int>(obj->getProperty("interval"));
        
        if (deviceCode.isEmpty() || userCode.isEmpty() || verificationUrl.isEmpty())
        {
            showError(TRANS("Incomplete device code response"));
            return;
        }
        
        updateUI();
        updateStatus(TRANS("Waiting for user authentication..."));
        progressValue = 0.3;
        progressBar->repaint();
        
        startTimer(interval * 1000);
    }
    catch (...)
    {
        showError(TRANS("Failed to parse device code response"));
    }
}

void R2CloudAuthComponent::updateUI()
{
    instructionLabel->setText(TRANS("1. Click 'Open URL' or go to the URL below\n2. Enter the code\n3. Complete authentication"),
                             juce::dontSendNotification);
    
    // コードを大きく表示
    codeDisplayLabel->setText(userCode, juce::dontSendNotification);
    
    // URLを見やすく表示
    urlDisplayLabel->setText(verificationUrl, juce::dontSendNotification);
    
    // ボタンを有効化
    openUrlButton->setEnabled(true);
    copyCodeButton->setEnabled(true);
    copyUrlButton->setEnabled(true);
    
    // 強制的に再描画
    repaint();
    
    // デバッグ出力でURLが正しく設定されているか確認
    DBG(juce::String("Verification URL: ") + verificationUrl);
    DBG(juce::String("User Code: ") + userCode);
    DBG(juce::String("URL Label bounds: ") + urlDisplayLabel->getBounds().toString());
    DBG(juce::String("URL Label text: ") + urlDisplayLabel->getText());
}

void R2CloudAuthComponent::timerCallback()
{
    if (deviceFlowActive)
        pollForAccessToken();
}

void R2CloudAuthComponent::pollForAccessToken()
{
    pollCount++;
    
    if (pollCount > maxPolls)
    {
        showError(TRANS("Authentication timeout"));
        return;
    }
    
    // プログレス更新
    progressValue = 0.3 + (static_cast<double>(pollCount) / maxPolls) * 0.6;
    progressBar->repaint();
    
    updateStatus(TRANS("Checking authentication... (") + juce::String(pollCount) + "/" + juce::String(maxPolls) + ")");
    
    juce::String postData = "client_id=" + juce::URL::addEscapeChars(googleClientId, false);
    postData += "&client_secret=" + juce::URL::addEscapeChars(googleClientSecret, false);
    postData += "&device_code=" + juce::URL::addEscapeChars(deviceCode, false);
    postData += "&grant_type=urn:ietf:params:oauth:grant-type:device_code";
    
    makeHttpRequest("https://oauth2.googleapis.com/token", postData,
        [this](bool success, juce::String response, int statusCode)
        {
            juce::MessageManager::callAsync([this, success, response]()
            {
                if (success)
                    parseTokenResponse(response);
            });
        });
}

void R2CloudAuthComponent::parseTokenResponse(const juce::String& response)
{
    try
    {
        auto json = juce::JSON::parse(response);
        if (!json.isObject())
            return;
        
        auto* obj = json.getDynamicObject();
        
        if (obj->hasProperty("access_token"))
        {
            auto accessToken = obj->getProperty("access_token").toString();
            auto refreshToken = obj->getProperty("refresh_token").toString();
            
            showSuccess();
            
            if (onAuthenticationComplete)
                onAuthenticationComplete(true, "", accessToken, refreshToken);  // ← トークンを渡す
                
            return;
        }
        else if (obj->hasProperty("error"))
        {
            auto error = obj->getProperty("error").toString();
            
            if (error == "authorization_pending")
            {
                // 継続
                return;
            }
            else if (error == "slow_down")
            {
                // レート制限 - 間隔を延長
                interval += 5;
                stopTimer();
                startTimer(interval * 1000);
                return;
            }
            else
            {
                // その他のエラー
                auto errorDesc = obj->getProperty("error_description").toString();
                showError(error + (errorDesc.isNotEmpty() ? ": " + errorDesc : ""));
                return;
            }
        }
    }
    catch (...)
    {
        // 継続
    }
}

void R2CloudAuthComponent::updateStatus(const juce::String& status)
{
    statusLabel->setText(status, juce::dontSendNotification);
}

void R2CloudAuthComponent::showError(const juce::String& error)
{
    stopTimer();
    deviceFlowActive = false;
    
    updateStatus(TRANS("Error: ") + error);
    progressValue = 0.0;
    progressBar->repaint();
    
    statusLabel->setColour(juce::Label::textColourId, juce::Colours::red);
    
    if (onAuthenticationComplete)
        onAuthenticationComplete(false, error, "", "");  // ← 空のトークンを渡す
}

void R2CloudAuthComponent::showSuccess()
{
    stopTimer();
    deviceFlowActive = false;
    
    updateStatus(TRANS("Authentication successful!"));
    progressValue = 1.0;
    progressBar->repaint();
    
    statusLabel->setColour(juce::Label::textColourId, juce::Colours::green);
    
    // 即座にstopAuthenticationを呼び、表示だけ1秒後に消す
    stopAuthentication();
    
    // 表示だけ1秒後に消す
    juce::Timer::callAfterDelay(1000, [safeThis = juce::Component::SafePointer<R2CloudAuthComponent>(this)]()
    {
        if (safeThis != nullptr)
        {
            safeThis->setVisible(false);
        }
    });
}

void R2CloudAuthComponent::buttonClicked(juce::Button* button)
{
    if (button == openUrlButton.get())
    {
        if (verificationUrl.isNotEmpty())
        {
            juce::URL(verificationUrl).launchInDefaultBrowser();
        }
    }
    else if (button == copyCodeButton.get())
    {
        copyToClipboard(userCode);
    }
    else if (button == copyUrlButton.get())
    {
        copyToClipboard(verificationUrl);
    }
    else if (button == cancelButton.get())
    {
        if (onAuthenticationCancelled)
            onAuthenticationCancelled();
        stopAuthentication();
    }
}

void R2CloudAuthComponent::copyToClipboard(const juce::String& text)
{
    juce::SystemClipboard::copyTextToClipboard(text);
    
    // 一時的にボタンテキストを変更してフィードバック
    if (text == userCode)
    {
        copyCodeButton->setButtonText(TRANS("Copied!"));
        juce::Timer::callAfterDelay(1000, [this]() { copyCodeButton->setButtonText(TRANS("Copy Code")); });
    }
    else if (text == verificationUrl)
    {
        copyUrlButton->setButtonText(TRANS("Copied!"));
        juce::Timer::callAfterDelay(1000, [this]() { copyUrlButton->setButtonText(TRANS("Copy URL")); });
    }
}

bool R2CloudAuthComponent::keyPressed(const juce::KeyPress& key)
{
    if (key.getKeyCode() == juce::KeyPress::escapeKey)
    {
        if (onAuthenticationCancelled)
            onAuthenticationCancelled();
        stopAuthentication();
        return true;
    }
    return false;
}

void R2CloudAuthComponent::paint(juce::Graphics& g)
{
    // 半透明の背景オーバーレイ
    g.fillAll(juce::Colours::black.withAlpha(0.8f));
    
    // 中央のコンテンツエリア
    auto bounds = getLocalBounds();
    auto contentArea = bounds.reduced(bounds.getWidth() / 8, bounds.getHeight() / 8);
    
    // コンテンツ背景
    g.setColour(juce::Colour(0xff2d2d2d));
    g.fillRoundedRectangle(contentArea.toFloat(), 10.0f);
    
    // 境界線
    g.setColour(juce::Colour(0xff4d4d4d));
    g.drawRoundedRectangle(contentArea.toFloat(), 10.0f, 2.0f);
    
    // URL表示エリアの背景を強調
    if (urlDisplayLabel->isVisible() && !urlDisplayLabel->getText().isEmpty())
    {
        auto urlBounds = urlDisplayLabel->getBounds();
        g.setColour(juce::Colours::darkblue);
        g.fillRoundedRectangle(urlBounds.toFloat(), 5.0f);
        g.setColour(juce::Colours::lightblue);
        g.drawRoundedRectangle(urlBounds.toFloat(), 5.0f, 1.0f);
    }
}

void R2CloudAuthComponent::resized()
{
   auto bounds = getLocalBounds();
   auto contentArea = bounds.reduced(bounds.getWidth() / 8, bounds.getHeight() / 8);
   
   auto area = contentArea.reduced(20);
   
   // タイトル (35px + 10px spacing)
   titleLabel->setBounds(area.removeFromTop(35));
   area.removeFromTop(10);
   
   // 指示テキスト (60px + 10px spacing)
   instructionLabel->setBounds(area.removeFromTop(60));
   area.removeFromTop(10);
   
   // コード表示 (45px + 10px spacing)
   codeDisplayLabel->setBounds(area.removeFromTop(45));
   area.removeFromTop(10);
   
   // URL表示 (30px + 10px spacing)
   urlDisplayLabel->setBounds(area.removeFromTop(30));
   area.removeFromTop(10);
   
   // ボタン行 (32px + 10px spacing)
   auto buttonArea = area.removeFromTop(32);
   auto buttonWidth = (buttonArea.getWidth() - 9) / 4; // 3つの間隔分(3px×3)を引く
   
   openUrlButton->setBounds(buttonArea.removeFromLeft(buttonWidth));
   buttonArea.removeFromLeft(3);
   copyCodeButton->setBounds(buttonArea.removeFromLeft(buttonWidth));
   buttonArea.removeFromLeft(3);
   copyUrlButton->setBounds(buttonArea.removeFromLeft(buttonWidth));
   buttonArea.removeFromLeft(3);
   cancelButton->setBounds(buttonArea.removeFromLeft(buttonWidth));
   
   area.removeFromTop(10);
   
   // ステータス (20px + 8px spacing)
   statusLabel->setBounds(area.removeFromTop(20));
   area.removeFromTop(8);
   
   // プログレスバー (15px)
   progressBar->setBounds(area.removeFromTop(15).reduced(30, 0));
}

void R2CloudAuthComponent::makeHttpRequest(const juce::String& url, const juce::String& postData,
                                          std::function<void(bool, juce::String, int)> callback)
{
    juce::Thread::launch([url, postData, callback]()
    {
        try
        {
            juce::URL requestUrl(url);
            
            if (postData.isNotEmpty())
            {
                requestUrl = requestUrl.withPOSTData(postData);
            }
            
            juce::StringPairArray headers;
            headers.set("Content-Type", "application/x-www-form-urlencoded");
            headers.set("Accept", "application/json");
            headers.set("User-Agent", "R2JUCE CloudAuth/1.0");
            
            juce::String headerString;
            for (int i = 0; i < headers.size(); ++i)
            {
                headerString += headers.getAllKeys()[i] + ": " + headers.getAllValues()[i];
                if (i < headers.size() - 1)
                    headerString += "\r\n";
            }

            auto webStream = std::make_unique<juce::WebInputStream>(requestUrl, !postData.isEmpty());
            webStream->withExtraHeaders(headerString);
            webStream->withConnectionTimeout(30000);
            webStream->withNumRedirectsToFollow(5);
            
            if (webStream->connect(nullptr))
            {
                auto response = webStream->readEntireStreamAsString();
                int statusCode = webStream->getStatusCode();
                bool success = (statusCode >= 200 && statusCode < 300);
                
                juce::MessageManager::callAsync([callback, response, statusCode, success]()
                {
                    if (callback)
                        callback(success, response, statusCode);
                });
            }
            else
            {
                juce::MessageManager::callAsync([callback]()
                {
                    if (callback)
                        callback(false, "Failed to connect to server", 0);
                });
            }
        }
        catch (const std::exception& e)
        {
            juce::MessageManager::callAsync([callback, e]()
            {
                if (callback)
                    callback(false, "HTTP request exception: " + juce::String(e.what()), 0);
            });
        }
        catch (...)
        {
            juce::MessageManager::callAsync([callback]()
            {
                if (callback)
                    callback(false, "Unknown HTTP request error", 0);
            });
        }
    });
}

} // namespace r2juce
