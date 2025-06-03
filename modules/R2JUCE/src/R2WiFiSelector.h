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

struct WifiNetwork
{
    juce::String ssid;
    int signal;
    bool isConnected = false;
    bool isSecured = true;
};

class ConnectingOverlay;
class PasswordInputOverlay;
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class R2WiFiSelector  : public juce::Component,
                        private juce::Timer,
                        juce::ListBoxModel,
                        public juce::Button::Listener
{
public:
    //==============================================================================
    R2WiFiSelector (std::function<void()> closeCallback);
    ~R2WiFiSelector() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    // ListBoxModel overrides
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, juce::Graphics& g,
                           int width, int height, bool rowIsSelected) override;
    void listBoxItemClicked (int row, const juce::MouseEvent&) override;

    // Callback from ConnectingOverlay
    void onConnectingCancelled();
    void onConnectingCompleted();
    void onPasswordEntered(const juce::String& password);
    void onPasswordCancelled();
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    void timerCallback() override;
    void scanNetworks();
    void connectToNetwork (const juce::String& ssid,
                           const juce::String& password);
    void updateConnectButton();
    void showConnectingOverlay();
    void hideConnectingOverlay();
    void showPasswordInputOverlay(const juce::String& ssid);
    void hidePasswordInputOverlay();
    void onConnectionResult(bool success, const juce::String& message);
    void drawWiFiStrengthArcs(juce::Graphics& g, int centerX, int centerY, int signalStrength);

    juce::OwnedArray<WifiNetwork> networks;
    int selectedNetworkIndex = -1;
    juce::String sudoPassword;
    juce::String previouslySelectedSSID;  // 更新後も選択を維持するため
    bool isConnecting = false;
    std::unique_ptr<ConnectingOverlay> connectingOverlay;
    std::unique_ptr<PasswordInputOverlay> passwordInputOverlay;
    std::unique_ptr<juce::ChildProcess> currentConnectionProcess;
    std::function<void()> onCloseCallback = nullptr;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::ListBox> listBox;
    std::unique_ptr<juce::Label> labelStatus;
    std::unique_ptr<juce::TextButton> buttonConnect;
    std::unique_ptr<juce::TextButton> buttonClose;
    std::unique_ptr<juce::Label> labelTitle;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2WiFiSelector)
};

//[EndFile] You can add extra defines here...
//==============================================================================
/**
    接続中オーバーレイコンポーネント
*/
class ConnectingOverlay : public juce::Component,
                          public juce::Button::Listener,
                          private juce::Timer
{
public:
    ConnectingOverlay(R2WiFiSelector* parent);
    ~ConnectingOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void timerCallback() override;  // この行を追加

    void setConnecting(const juce::String& ssid);
    void setFailed(const juce::String& message);

private:
    R2WiFiSelector* parentSelector;
    std::unique_ptr<juce::Label> statusLabel;
    std::unique_ptr<juce::TextButton> actionButton;
    bool isFailedState = false;
    int spinnerIndex = 0;  // この行を追加
    juce::String currentSSID;  // この行を追加

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ConnectingOverlay)
};

//==============================================================================
/**
    パスワード入力オーバーレイコンポーネント
*/
class PasswordInputOverlay : public juce::Component,
                            public juce::Button::Listener,
                            public juce::TextEditor::Listener
{
public:
    PasswordInputOverlay(R2WiFiSelector* parent);
    ~PasswordInputOverlay() override;

    void paint(juce::Graphics& g) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;
    void textEditorReturnKeyPressed(juce::TextEditor& editor) override;

    void setSSID(const juce::String& ssid);
    void grabPasswordFocus();

private:
    R2WiFiSelector* parentSelector;
    std::unique_ptr<juce::Label> titleLabel;
    std::unique_ptr<juce::Label> ssidLabel;
    std::unique_ptr<juce::Label> passwordLabel;
    std::unique_ptr<juce::TextEditor> passwordEditor;
    std::unique_ptr<juce::TextButton> connectButton;
    std::unique_ptr<juce::TextButton> cancelButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PasswordInputOverlay)
};
//[/EndFile]

