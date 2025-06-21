/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.12

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "R2CloudAuthComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
namespace r2juce {
//[/MiscUserDefs]

//==============================================================================
R2CloudAuthComponent::R2CloudAuthComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    labelTitle.reset (new juce::Label (juce::String(),
                                       TRANS ("Title")));
    addAndMakeVisible (labelTitle.get());
    labelTitle->setFont (juce::Font (juce::FontOptions (22.00f, juce::Font::plain)));
    labelTitle->setJustificationType (juce::Justification::centred);
    labelTitle->setEditable (false, false, false);
    labelTitle->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelTitle->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelInstruction.reset (new juce::Label (juce::String(),
                                             TRANS ("1. Click URL or go to the URL below\n"
                                             "2. Enter the code\n"
                                             "3. Complete authentication")));
    addAndMakeVisible (labelInstruction.get());
    labelInstruction->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelInstruction->setJustificationType (juce::Justification::centred);
    labelInstruction->setEditable (false, false, false);
    labelInstruction->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelInstruction->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelCodeDisplay.reset (new juce::Label (juce::String(),
                                             TRANS ("XXX-XXX-XXX")));
    addAndMakeVisible (labelCodeDisplay.get());
    labelCodeDisplay->setFont (juce::Font (juce::FontOptions (32.00f, juce::Font::plain)));
    labelCodeDisplay->setJustificationType (juce::Justification::centred);
    labelCodeDisplay->setEditable (false, false, false);
    labelCodeDisplay->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelCodeDisplay->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    buttonURL.reset (new juce::HyperlinkButton (TRANS ("https://www.xxx.com"),
                                                juce::URL ("http://www.juce.com")));
    addAndMakeVisible (buttonURL.get());
    buttonURL->setTooltip (TRANS ("http://www.juce.com"));
    buttonURL->setButtonText (TRANS ("https://www.xxx.com"));

    buttonCancel.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonCancel.get());
    buttonCancel->setButtonText (TRANS ("Cancel"));
    buttonCancel->addListener (this);

    buttonCopyUrl.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonCopyUrl.get());
    buttonCopyUrl->setButtonText (TRANS ("Copy URL"));
    buttonCopyUrl->addListener (this);

    buttonCopyCode.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonCopyCode.get());
    buttonCopyCode->setButtonText (TRANS ("Copy Code"));
    buttonCopyCode->addListener (this);

    labelStatus.reset (new juce::Label (juce::String(),
                                        TRANS ("Status")));
    addAndMakeVisible (labelStatus.get());
    labelStatus->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelStatus->setJustificationType (juce::Justification::centredLeft);
    labelStatus->setEditable (false, false, false);
    labelStatus->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelStatus->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelStatus->setBounds (16, 240, 64, 24);

    progressBar.reset (new juce::ProgressBar (progressValue));
    addAndMakeVisible (progressBar.get());


    //[UserPreSize]
    setWantsKeyboardFocus (true);

    buttonURL->setEnabled (false);
    buttonCopyUrl->setEnabled (false);
    buttonCopyCode->setEnabled (false);

    buttonURL->setButtonText (TRANS("Waiting for code..."));
    buttonURL->setTooltip (TRANS("Waiting for code..."));
    labelCodeDisplay->setText (TRANS("Waiting for code..."), juce::dontSendNotification);

    //setVisible (false);
    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

R2CloudAuthComponent::~R2CloudAuthComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    stopAuthentication();
    //[/Destructor_pre]

    labelTitle = nullptr;
    labelInstruction = nullptr;
    labelCodeDisplay = nullptr;
    buttonURL = nullptr;
    buttonCancel = nullptr;
    buttonCopyUrl = nullptr;
    buttonCopyCode = nullptr;
    labelStatus = nullptr;
    progressBar = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void R2CloudAuthComponent::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void R2CloudAuthComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    labelTitle->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 8, getWidth() - 32, 32);
    labelInstruction->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 40, getWidth() - 32, 56);
    labelCodeDisplay->setBounds ((getWidth() / 2) - (536 / 2), 168, 536, 32);
    buttonURL->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 104, getWidth() - 32, 24);
    buttonCancel->setBounds ((getWidth() / 2) - (128 / 2), 280, 128, 24);
    buttonCopyUrl->setBounds ((getWidth() / 2) - (128 / 2), 136, 128, 24);
    buttonCopyCode->setBounds ((getWidth() / 2) - (128 / 2), 208, 128, 24);
    progressBar->setBounds (getWidth() - 16 - (getWidth() - 96), 240, getWidth() - 96, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void R2CloudAuthComponent::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == buttonCancel.get())
    {
        //[UserButtonCode_buttonCancel] -- add your button handler code here..
        if (onAuthenticationCancelled)
            onAuthenticationCancelled();
        stopAuthentication();
        //[/UserButtonCode_buttonCancel]
    }
    else if (buttonThatWasClicked == buttonCopyUrl.get())
    {
        //[UserButtonCode_buttonCopyUrl] -- add your button handler code here..
        copyToClipboard(verificationUrl);
        //[/UserButtonCode_buttonCopyUrl]
    }
    else if (buttonThatWasClicked == buttonCopyCode.get())
    {
        //[UserButtonCode_buttonCopyCode] -- add your button handler code here..
        copyToClipboard (userCode);
        //[/UserButtonCode_buttonCopyCode]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
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
            labelTitle->setText (TRANS("Google Drive Authentication"), juce::dontSendNotification);
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
//    instructionLabel->setText(TRANS("1. Click 'Open URL' or go to the URL below\n2. Enter the code\n3. Complete authentication"),
//                             juce::dontSendNotification);
    
    labelCodeDisplay->setText (userCode, juce::dontSendNotification);
    buttonURL->setButtonText(verificationUrl);
    buttonURL->setTooltip (verificationUrl);
    buttonURL->setURL (verificationUrl);
    
    buttonURL->setEnabled(true);
    buttonCopyUrl->setEnabled (true);
    buttonCopyCode->setEnabled (true);
    
    repaint();
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
    labelStatus->setText (status, juce::dontSendNotification);
}

void R2CloudAuthComponent::showError(const juce::String& error)
{
    stopTimer();
    deviceFlowActive = false;
    
    updateStatus(TRANS("Error: ") + error);
    progressValue = 0.0;
    progressBar->repaint();
    
    labelStatus->setColour(juce::Label::textColourId, juce::Colours::red);
    
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
    
    labelStatus->setColour(juce::Label::textColourId, juce::Colours::green);
    
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

void R2CloudAuthComponent::copyToClipboard(const juce::String& text)
{
    juce::SystemClipboard::copyTextToClipboard(text);
    
    // 一時的にボタンテキストを変更してフィードバック
    if (text == userCode)
    {
        auto prevText = buttonCopyCode->getButtonText();
        buttonCopyCode->setButtonText (TRANS("Copied!"));
        juce::Timer::callAfterDelay (1000, [this, prevText]() { buttonCopyCode->setButtonText(prevText); });
    }
    else if (text == verificationUrl)
    {
        auto prevText = buttonCopyUrl->getButtonText();
        buttonCopyUrl->setButtonText(TRANS("Copied!"));
        juce::Timer::callAfterDelay(1000, [this, prevText]() { buttonCopyUrl->setButtonText(prevText); });
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

}   //  namespace r2juce
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="R2CloudAuthComponent" componentName=""
                 parentClasses="public juce::Component, public juce::Timer" constructorParams=""
                 variableInitialisers="" snapPixels="8" snapActive="1" snapShown="1"
                 overlayOpacity="0.330" fixedSize="1" initialWidth="568" initialHeight="320">
  <BACKGROUND backgroundColour="ff323e44"/>
  <LABEL name="" id="95463433c749ef8" memberName="labelTitle" virtualName=""
         explicitFocusOrder="0" pos="0Cc 8 32M 32" edTextCol="ff000000"
         edBkgCol="0" labelText="Title" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="22.0"
         kerning="0.0" bold="1" italic="0" justification="36" typefaceStyle="Bold"/>
  <LABEL name="" id="e34e1b93f56c2849" memberName="labelInstruction" virtualName=""
         explicitFocusOrder="0" pos="0Cc 40 32M 56" edTextCol="ff000000"
         edBkgCol="0" labelText="1. Click URL or go to the URL below&#10;2. Enter the code&#10;3. Complete authentication"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="36"/>
  <LABEL name="" id="88e2d4137011f05b" memberName="labelCodeDisplay" virtualName=""
         explicitFocusOrder="0" pos="0Cc 168 536 32" edTextCol="ff000000"
         edBkgCol="0" labelText="XXX-XXX-XXX" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="32.0" kerning="0.0" bold="0" italic="0" justification="36"/>
  <HYPERLINKBUTTON name="" id="3ff1b1e361014b63" memberName="buttonURL" virtualName=""
                   explicitFocusOrder="0" pos="0Cc 104 32M 24" tooltip="http://www.juce.com"
                   buttonText="https://www.xxx.com" connectedEdges="0" needsCallback="0"
                   radioGroupId="0" url="http://www.juce.com"/>
  <TEXTBUTTON name="" id="4c5029f7e0459d47" memberName="buttonCancel" virtualName=""
              explicitFocusOrder="0" pos="0Cc 280 128 24" buttonText="Cancel"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="b784255939e91b89" memberName="buttonCopyUrl" virtualName=""
              explicitFocusOrder="0" pos="0Cc 136 128 24" buttonText="Copy URL"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="c06ddf2a594e77d7" memberName="buttonCopyCode" virtualName=""
              explicitFocusOrder="0" pos="0Cc 208 128 24" buttonText="Copy Code"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="b6ec3558c7c0f3d" memberName="labelStatus" virtualName=""
         explicitFocusOrder="0" pos="16 240 64 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Status" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="" id="2452a8ab4cd2a84c" memberName="progressBar" virtualName=""
                    explicitFocusOrder="0" pos="16Rr 240 96M 24" class="juce::ProgressBar"
                    params="progressValue"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

