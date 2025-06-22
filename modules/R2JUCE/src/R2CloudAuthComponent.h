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

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
#include <functional>

namespace r2juce {
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
 An auto-generated component, created by the Projucer.

 Describe your class and how it works here!
                                                                    //[/Comments]
*/
class R2CloudAuthComponent  : public juce::Component,
                              public juce::Timer,
                              public juce::Button::Listener
{
public:
    //==============================================================================
    R2CloudAuthComponent ();
    ~R2CloudAuthComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    enum class ServiceType
    {
        GoogleDrive,
        OneDrive
    };

    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);
    void setServiceType(ServiceType type);

    void startAuthentication();
    void stopAuthentication();

    std::function<void(bool success, juce::String errorMessage, juce::String accessToken, juce::String refreshToken)> onAuthenticationComplete;
    std::function<void()> onAuthenticationCancelled;

    void timerCallback() override;

    bool keyPressed(const juce::KeyPress& key) override;
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    // Internal methods
    void setupUI();
    void requestDeviceCode();
    void pollForAccessToken();
    void parseDeviceCodeResponse(const juce::String& response);
    void parseTokenResponse(const juce::String& response);
    void updateUI();
    void updateStatus(const juce::String& status);
    void showError(const juce::String& error);
    void showSuccess();
    void copyToClipboard(const juce::String& text);

    // HTTP Request
    void makeHttpRequest(const juce::String& url, const juce::String& postData,
                         std::function<void(bool, juce::String, int)> callback);

    ServiceType serviceType = ServiceType::GoogleDrive;

    juce::String googleClientId;
    juce::String googleClientSecret;

    juce::String oneDriveClientId;
    juce::String oneDriveClientSecret;

    bool deviceFlowActive = false;
    juce::String deviceCode;
    juce::String userCode;
    juce::String verificationUrl;
    int pollCount = 0;
    int interval = 5;
    static constexpr int maxPolls = 120;
    double progressValue = 0.0;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Label> labelTitle;
    std::unique_ptr<juce::Label> labelInstruction;
    std::unique_ptr<juce::Label> labelCodeDisplay;
    std::unique_ptr<juce::HyperlinkButton> buttonURL;
    std::unique_ptr<juce::TextButton> buttonCancel;
    std::unique_ptr<juce::TextButton> buttonCopyUrl;
    std::unique_ptr<juce::TextButton> buttonCopyCode;
    std::unique_ptr<juce::Label> labelStatus;
    std::unique_ptr<juce::ProgressBar> progressBar;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2CloudAuthComponent)
};

//[EndFile] You can add extra defines here...
}   //  namespace r2juce
//[/EndFile]

