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
  @brief A GUI component for handling cloud service authentication using the OAuth 2.0 Device Authorization Flow.

  @details This component provides a user interface for services like Google Drive and
           OneDrive. It guides the user to visit a URL, enter a code, and authorize
           the application on their device. The process is handled asynchronously,
           with results delivered via public callbacks.
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

    /** @brief Specifies the cloud service to be authenticated. */
    enum class ServiceType
    {
        GoogleDrive,
        OneDrive
    };

    /** @brief Sets the Client ID and Client Secret for the Google Drive API.
        @param clientId     The OAuth 2.0 Client ID obtained from the Google API Console.
        @param clientSecret The OAuth 2.0 Client Secret.
    */
    void setGoogleCredentials(const juce::String& clientId, const juce::String& clientSecret);

    /** @brief Sets the Client ID and Client Secret for the Microsoft OneDrive API.
        @param clientId     The Application (client) ID from the Azure portal.
        @param clientSecret The client secret value.
    */
    void setOneDriveCredentials(const juce::String& clientId, const juce::String& clientSecret);

    /** @brief Sets the target cloud service for authentication. This should be called before startAuthentication().
        @param type The service to use (GoogleDrive or OneDrive).
    */
    void setServiceType(ServiceType type);

    /** @brief Begins the device authentication flow.
        @details Makes the component visible and starts the process of requesting a device
                 and user code from the selected cloud service.
    */
    void startAuthentication();

    /** @brief Stops the authentication process immediately.
        @details Halts the polling timer, clears internal state, and hides the component.
                 Triggers the onAuthenticationCancelled callback if it is set.
    */
    void stopAuthentication();

    /** @brief A callback function that is triggered when the authentication process concludes.
        @details The callback provides a success flag, an error message on failure,
                 and the access and refresh tokens on success.
                 - `success`: True if authentication succeeded, false otherwise.
                 - `errorMessage`: A description of the error if authentication failed.
                 - `accessToken`: The token used to make authorized API calls.
                 - `refreshToken`: The token used to obtain a new access token.
    */
    std::function<void(bool success, juce::String errorMessage, juce::String accessToken, juce::String refreshToken)> onAuthenticationComplete;

    /** @brief A callback function that is triggered when the user cancels the authentication
               by clicking the cancel button or pressing the Escape key.
    */
    std::function<void()> onAuthenticationCancelled;

    /** @brief The timer callback, used to periodically poll for the access token. (Inherited from juce::Timer). */
    void timerCallback() override;

    /** @brief Handles key presses. (Inherited from juce::Component).
        @details Listens for the Escape key to cancel the authentication process.
    */
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

