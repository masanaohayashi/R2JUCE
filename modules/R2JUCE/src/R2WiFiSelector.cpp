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

#include "R2WiFiSelector.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
namespace r2juce {
//[/MiscUserDefs]

//==============================================================================
R2WiFiSelector::R2WiFiSelector (std::function<void()> closeCallback)
    : onCloseCallback(closeCallback)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    listBox.reset (new juce::ListBox ("WiFiListBox", this));
    addAndMakeVisible (listBox.get());

    labelStatus.reset (new juce::Label (juce::String(),
                                        TRANS ("(status)")));
    addAndMakeVisible (labelStatus.get());
    labelStatus->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelStatus->setJustificationType (juce::Justification::centredLeft);
    labelStatus->setEditable (false, false, false);
    labelStatus->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelStatus->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    buttonConnect.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonConnect.get());
    buttonConnect->setButtonText (TRANS ("Connect"));
    buttonConnect->addListener (this);

    buttonClose.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonClose.get());
    buttonClose->setButtonText (TRANS ("X"));
    buttonClose->addListener (this);

    labelTitle.reset (new juce::Label (juce::String(),
                                       TRANS ("Wi-Fi Connection")));
    addAndMakeVisible (labelTitle.get());
    labelTitle->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelTitle->setJustificationType (juce::Justification::centredLeft);
    labelTitle->setEditable (false, false, false);
    labelTitle->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelTitle->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));


    //[UserPreSize]
    buttonClose->setVisible(onCloseCallback != nullptr);
    listBox->setModel (this);
    listBox->setRowHeight (24);
    listBox->setMultipleSelectionEnabled (false);
    labelStatus->setText ("", juce::dontSendNotification);
    updateConnectButton();
    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    /*
     // Test on Mac
     passwordInputOverlay.reset(new PasswordInputOverlay(this));
     addAndMakeVisible(passwordInputOverlay.get());
     passwordInputOverlay->setBounds(getLocalBounds());
     passwordInputOverlay->setSSID("Test SSID");

     connectingOverlay.reset (new ConnectingOverlay (this));
     addAndMakeVisible (connectingOverlay.get());
     connectingOverlay->setBounds (getLocalBounds());
     */

    startTimer (3000);
    scanNetworks();
    //[/Constructor]
}

R2WiFiSelector::~R2WiFiSelector()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    stopTimer();
    connectingOverlay = nullptr;
    //[/Destructor_pre]

    listBox = nullptr;
    labelStatus = nullptr;
    buttonConnect = nullptr;
    buttonClose = nullptr;
    labelTitle = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void R2WiFiSelector::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void R2WiFiSelector::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    listBox->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 40, getWidth() - 32, getHeight() - 120);
    labelStatus->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), getHeight() - 8 - 24, getWidth() - 32, 24);
    buttonConnect->setBounds ((getWidth() / 2) - (152 / 2), getHeight() - 40 - 24, 152, 24);
    buttonClose->setBounds (getWidth() - 16 - 24, 8, 24, 24);
    labelTitle->setBounds (24, 8, getWidth() - 74, 24);
    //[UserResized] Add your own custom resize handling here..
    if (connectingOverlay != nullptr)
        connectingOverlay->setBounds (getLocalBounds());
    //[/UserResized]
}

void R2WiFiSelector::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == buttonConnect.get())
    {
        //[UserButtonCode_buttonConnect] -- add your button handler code here..
        if (selectedNetworkIndex >= 0 && selectedNetworkIndex < networks.size() && !isConnecting)
        {
            auto* selectedNet = networks[selectedNetworkIndex];

            if (selectedNet->isSecured)
            {
                showPasswordInputOverlay(selectedNet->ssid);
            }
            else
            {
                connectToNetwork(selectedNet->ssid, "");
            }
        }
        //[/UserButtonCode_buttonConnect]
    }
    else if (buttonThatWasClicked == buttonClose.get())
    {
        //[UserButtonCode_buttonClose] -- add your button handler code here..
        if (onCloseCallback)
        {
            onCloseCallback();
        }
        else
        {
            getParentComponent()->removeChildComponent(this);
            delete this;
        }
        //[/UserButtonCode_buttonClose]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
int R2WiFiSelector::getNumRows()
{
    return networks.size();
}

void R2WiFiSelector::paintListBoxItem (int rowNumber, juce::Graphics& g,
                                       int width, int height, bool rowIsSelected)
{
    // Background
    if (rowIsSelected)
        g.fillAll (juce::Colours::lightblue.withAlpha (0.8f));
    else if (rowNumber % 2 == 0)
        g.fillAll (juce::Colour (0xff404040));
    else
        g.fillAll (juce::Colour (0xff383838));

    if (auto* net = networks[rowNumber])
    {
        // Network name and status
        juce::String text = net->ssid;
        if (net->isConnected)
            text += " (Connected)";

        g.setColour (net->isConnected ? juce::Colours::lightgreen : juce::Colours::white);
        g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::plain)));
        g.drawText (text, 8, 0, width - 80, height, juce::Justification::centredLeft);

        // Security indicator
        if (net->isSecured)
        {
            g.setColour (juce::Colours::orange);
            g.setFont (juce::Font (juce::FontOptions (10.0f, juce::Font::plain)));
            g.drawText ("P", width - 70, 0, 15, height, juce::Justification::centred);
        }

        // WiFi signal strength (Mac-style arcs)
        drawWiFiStrengthArcs(g, width - 50, height / 2, net->signal);
    }
}

void R2WiFiSelector::listBoxItemClicked (int row, const juce::MouseEvent&)
{
    if (row < 0 || row >= networks.size() || isConnecting)
        return;

    selectedNetworkIndex = row;

    if (auto* net = networks[selectedNetworkIndex])
        previouslySelectedSSID = net->ssid;

    updateConnectButton();
    listBox->repaint();
}

void R2WiFiSelector::onConnectingCancelled()
{
    if (currentConnectionProcess != nullptr)
    {
        currentConnectionProcess->kill();
        currentConnectionProcess = nullptr;
    }

    isConnecting = false;
    hideConnectingOverlay();
    updateConnectButton();
    labelStatus->setText ("Connection cancelled", juce::dontSendNotification);
}

void R2WiFiSelector::onConnectingCompleted()
{
    hideConnectingOverlay();
}

void R2WiFiSelector::onPasswordEntered(const juce::String& password)
{
    if (selectedNetworkIndex >= 0 && selectedNetworkIndex < networks.size())
    {
        auto* selectedNet = networks[selectedNetworkIndex];
        hidePasswordInputOverlay();
        connectToNetwork(selectedNet->ssid, password);
    }
}

void R2WiFiSelector::onPasswordCancelled()
{
    hidePasswordInputOverlay();
}

void R2WiFiSelector::timerCallback()
{
    if (!isConnecting)
        scanNetworks();
}

void R2WiFiSelector::scanNetworks()
{
    labelStatus->setText ("Scanning networks...", juce::dontSendNotification);

    juce::ChildProcess proc;

    if (!proc.start ("sudo nmcli device wifi rescan"))
    {
        labelStatus->setText ("Failed to rescan WiFi", juce::dontSendNotification);
        return;
    }

    proc.waitForProcessToFinish (3000);

    if (!proc.start ("sudo nmcli -t -f SSID,SIGNAL,SECURITY,ACTIVE device wifi list"))
    {
        labelStatus->setText ("Failed to list WiFi networks", juce::dontSendNotification);
        return;
    }

    proc.waitForProcessToFinish (5000);
    juce::String output = proc.readAllProcessOutput().trim();

    if (output.isEmpty())
    {
        labelStatus->setText ("No networks found", juce::dontSendNotification);
        return;
    }

    if (selectedNetworkIndex >= 0 && selectedNetworkIndex < networks.size())
        previouslySelectedSSID = networks[selectedNetworkIndex]->ssid;

    networks.clear();
    selectedNetworkIndex = -1;

    auto lines = juce::StringArray::fromLines (output);

    for (auto& line : lines)
    {
        auto parts = juce::StringArray::fromTokens (line, ":", "");
        if (parts.size() >= 4)
        {
            juce::String ssid = parts[0];
            int signal = parts[1].getIntValue();
            juce::String security = parts[2];
            juce::String active = parts[3];

            if (ssid.isEmpty())
                continue;

            auto* net = new WifiNetwork();
            net->ssid = ssid;
            net->signal = signal;
            net->isSecured = !security.isEmpty() && security != "--";
            net->isConnected = (active == "yes");
            networks.add (net);
        }
    }

#if JUCE_MAC
    {
        auto* net = new WifiNetwork();
        net->ssid = "Test SSID";
        net->signal = 10;
        net->isSecured = true;
        net->isConnected = false;
        networks.add (net);
    }
#endif

    struct NetworkComparator
    {
        int compareElements (WifiNetwork* a, WifiNetwork* b) const
        {
            if (a->isConnected != b->isConnected)
                return a->isConnected ? -1 : 1;
            return a->ssid.compareIgnoreCase (b->ssid);
        }
    };

    NetworkComparator comparator;
    networks.sort (comparator);

    if (previouslySelectedSSID.isNotEmpty())
    {
        for (int i = 0; i < networks.size(); ++i)
        {
            if (networks[i]->ssid == previouslySelectedSSID)
            {
                selectedNetworkIndex = i;
                break;
            }
        }
    }

    listBox->updateContent();
    listBox->repaint();

    labelStatus->setText (juce::String (networks.size()) + " networks found", juce::dontSendNotification);
    updateConnectButton();
}

void R2WiFiSelector::connectToNetwork (const juce::String& ssid, const juce::String& password)
{
    isConnecting = true;
    updateConnectButton();
    showConnectingOverlay();

    juce::Thread::launch ([this, ssid, password]()
                          {
        DBG("=== WiFi Connection Debug ===");
        DBG("Timestamp: " + juce::Time::getCurrentTime().toString(true, true));
        DBG("SSID: " + ssid);
        DBG("Password provided: " + juce::String(password.isNotEmpty() ? "Yes" : "No"));
        DBG("");

        bool success = false;
        juce::String message;

        DBG("Step 1: Disconnecting preconfigured connection...");
        juce::ChildProcess disconnectProc;
        if (disconnectProc.start("sudo nmcli connection down preconfigured"))
        {
            disconnectProc.waitForProcessToFinish(5000);
            auto disconnectOutput = disconnectProc.readAllProcessOutput();
            DBG("Disconnect output: " + disconnectOutput);
        }

        juce::Thread::sleep(2000);

        DBG("Step 2: Connecting to new network...");
        currentConnectionProcess.reset(new juce::ChildProcess());

        juce::String connectCmd;
        if (password.isEmpty())
        {
            connectCmd = "sudo nmcli device wifi connect " + ssid;
        }
        else
        {
            connectCmd = "sudo nmcli device wifi connect " + ssid + " password " + password;
        }

        DBG("Connect command: " + connectCmd);

        if (currentConnectionProcess->start(connectCmd))
        {
            DBG("Connection process started");

            bool finished = currentConnectionProcess->waitForProcessToFinish(15000);
            auto output = currentConnectionProcess->readAllProcessOutput().trim();
            int exitCode = currentConnectionProcess->getExitCode();

            DBG("Process finished: " + juce::String(finished ? "Yes" : "No"));
            DBG("Exit code: " + juce::String(exitCode));
            DBG("Output: " + output);

            if (!finished)
            {
                DBG("Connection timeout - killing process");
                currentConnectionProcess->kill();
                success = false;
                message = "Connection timeout";
            }
            else if (exitCode == 0)
            {
                success = true;
                message = "Successfully connected to: " + ssid;
            }
            else
            {
                success = false;
                if (output.contains("802.1X supplicant failed") || output.contains("supplicant failed"))
                {
                    message = "Authentication failed - incorrect password";
                }
                else if (output.contains("No network with SSID"))
                {
                    message = "Network not found: " + ssid;
                }
                else if (output.contains("Connection activation failed"))
                {
                    message = "Connection activation failed";
                }
                else
                {
                    message = "Connection failed: " + output.substring(0, 50);
                }
            }
        }
        else
        {
            DBG("Failed to start connection process");
            success = false;
            message = "Failed to start connection process";
        }

        if (success)
        {
            DBG("Step 3: Verifying connection...");
            juce::Thread::sleep(2000);

            juce::ChildProcess verifyProc;
            verifyProc.start("sudo nmcli connection show --active");
            verifyProc.waitForProcessToFinish(3000);
            auto activeConnections = verifyProc.readAllProcessOutput();
            DBG("Active connections after connect:");
            DBG(activeConnections);
        }

        currentConnectionProcess = nullptr;

        DBG("");
        DBG(juce::String("Final result: ") + (success ? "Success" : "Failed"));
        DBG("Message: " + message);
        DBG("=== End WiFi Connection Debug ===");

        if (isConnecting)
        {
            juce::MessageManager::callAsync ([this, success, message]()
                                             {
                onConnectionResult (success, message);
            });
        }
    });
}


void R2WiFiSelector::updateConnectButton()
{
    buttonConnect->setEnabled(selectedNetworkIndex >= 0 && selectedNetworkIndex < networks.size() && !isConnecting);
}

void R2WiFiSelector::showConnectingOverlay()
{
    if (connectingOverlay == nullptr)
    {
        connectingOverlay.reset (new ConnectingOverlay (this));
        addAndMakeVisible (connectingOverlay.get());
        connectingOverlay->setBounds (getLocalBounds());
    }

    if (selectedNetworkIndex >= 0 && selectedNetworkIndex < networks.size())
    {
        connectingOverlay->setConnecting (networks[selectedNetworkIndex]->ssid);
    }
}

void R2WiFiSelector::hideConnectingOverlay()
{
    connectingOverlay = nullptr;
}

void R2WiFiSelector::showPasswordInputOverlay(const juce::String& ssid)
{
    if (passwordInputOverlay == nullptr)
    {
        passwordInputOverlay.reset(new PasswordInputOverlay(this));
        addAndMakeVisible(passwordInputOverlay.get());
        passwordInputOverlay->setBounds(getLocalBounds());
    }

    passwordInputOverlay->setSSID(ssid);
    passwordInputOverlay->grabPasswordFocus();
}

void R2WiFiSelector::hidePasswordInputOverlay()
{
    passwordInputOverlay = nullptr;
}

void R2WiFiSelector::onConnectionResult(bool success, const juce::String& message)
{
    isConnecting = false;

    if (success)
    {
        hideConnectingOverlay();
        labelStatus->setText (message, juce::dontSendNotification);
        scanNetworks();
    }
    else
    {
        if (connectingOverlay != nullptr)
            connectingOverlay->setFailed (message);
    }

    updateConnectButton();
}

void R2WiFiSelector::drawWiFiStrengthArcs(juce::Graphics& g, int centerX, int centerY, int signalStrength)
{
    const int barCount = 3;
    const int barWidth = 2;
    const int barSpacing = 3;
    const int maxBarHeight = 10;

    int whiteBars = 0;
    if (signalStrength >= 33)
        whiteBars = 1;
    if (signalStrength >= 66)
        whiteBars = 2;
    if (signalStrength >= 90)
        whiteBars = 3;

    for (int i = 0; i < barCount; ++i)
    {
        int barHeight = 4 + (i * 3);
        int x = centerX - ((barCount - 1) * (barWidth + barSpacing) / 2) + i * (barWidth + barSpacing);
        int y = centerY + (maxBarHeight / 2) - barHeight;

        juce::Colour barColour;
        if (i < whiteBars)
        {
            barColour = juce::Colours::white;
        }
        else
        {
            barColour = juce::Colours::black;
        }

        g.setColour(barColour);
        g.fillRect(x, y, barWidth, barHeight);
    }
}

//==============================================================================
// ConnectingOverlay Implementation
//==============================================================================
ConnectingOverlay::ConnectingOverlay(R2WiFiSelector* parent)
: parentSelector(parent), isFailedState(false), spinnerIndex(0)
{
    // Status label
    statusLabel.reset (new juce::Label (juce::String(), "Connecting..."));
    addAndMakeVisible (statusLabel.get());
    statusLabel->setFont (juce::Font (juce::FontOptions (24.00f, juce::Font::plain)));
    statusLabel->setJustificationType (juce::Justification::centred);
    statusLabel->setColour (juce::Label::textColourId, juce::Colours::white);
    statusLabel->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    // Action button (Cancel/OK)
    actionButton.reset (new juce::TextButton ("Cancel"));
    addAndMakeVisible (actionButton.get());
    actionButton->addListener (this);

    startTimer(500);
}

ConnectingOverlay::~ConnectingOverlay()
{
    statusLabel = nullptr;
    actionButton = nullptr;
}

void ConnectingOverlay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xc0000000));
}

void ConnectingOverlay::resized()
{
    auto bounds = getLocalBounds();

    statusLabel->setBounds(16, 16, bounds.getWidth() - 32, 32);

    actionButton->setBounds((bounds.getWidth() / 2) - (128 / 2), 160, 128, 24);
}

void ConnectingOverlay::buttonClicked(juce::Button* button)
{
    if (button == actionButton.get())
    {
        if (isFailedState)
        {
            parentSelector->onConnectingCompleted();
        }
        else
        {
            parentSelector->onConnectingCancelled();
        }
    }
}

void ConnectingOverlay::timerCallback()
{
    if (!isFailedState)
    {
        juce::StringArray spinChars = {"|", "/", "-", "\\"};
        spinnerIndex = (spinnerIndex + 1) % spinChars.size();

        juce::String baseText = currentSSID.isNotEmpty() ?
        "Connecting to: " + currentSSID + " " : "Connecting... ";

        statusLabel->setText(baseText + spinChars[spinnerIndex], juce::dontSendNotification);
    }
}

void ConnectingOverlay::setConnecting(const juce::String& ssid)
{
    isFailedState = false;
    statusLabel->setText ("Connecting to: " + ssid, juce::dontSendNotification);
    actionButton->setButtonText ("Cancel");

    startTimer(500);
}

void ConnectingOverlay::setFailed(const juce::String& message)
{
    isFailedState = true;
    statusLabel->setText ("Connection Failed", juce::dontSendNotification);
    actionButton->setButtonText ("OK");
    stopTimer();
}

//==============================================================================
// PasswordInputOverlay Implementation
//==============================================================================

PasswordInputOverlay::PasswordInputOverlay(R2WiFiSelector* parent)
: parentSelector(parent)
{
    // Title label
    titleLabel.reset(new juce::Label(juce::String(), "Enter WiFi Password"));
    addAndMakeVisible(titleLabel.get());
    titleLabel->setFont(juce::Font(juce::FontOptions(24.00f, juce::Font::plain)));
    titleLabel->setJustificationType(juce::Justification::centred);
    titleLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

    // SSID label
    ssidLabel.reset(new juce::Label(juce::String(), "SSID: "));
    addAndMakeVisible(ssidLabel.get());
    ssidLabel->setFont(juce::Font(juce::FontOptions(15.00f, juce::Font::plain)));
    ssidLabel->setJustificationType(juce::Justification::centred);
    ssidLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    ssidLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

    // Password label
    passwordLabel.reset(new juce::Label(juce::String(), "Password:"));
    addAndMakeVisible(passwordLabel.get());
    passwordLabel->setFont(juce::Font(juce::FontOptions(15.00f, juce::Font::plain)));
    passwordLabel->setJustificationType(juce::Justification::centredLeft);
    passwordLabel->setColour(juce::Label::textColourId, juce::Colours::white);
    passwordLabel->setColour(juce::TextEditor::backgroundColourId, juce::Colour(0x00000000));

    // Password editor
    passwordEditor.reset(new R2TextEditor("passwordInput"));
    addAndMakeVisible(passwordEditor.get());
    //passwordEditor->setPasswordCharacter('*');
    passwordEditor->addListener(this);
    passwordEditor->setMultiLine(false);
    passwordEditor->setReturnKeyStartsNewLine(false);
    passwordEditor->setReadOnly(false);
    passwordEditor->setScrollbarsShown(true);
    passwordEditor->setCaretVisible(true);
    passwordEditor->setPopupMenuEnabled(true);

    // Connect button
    connectButton.reset(new juce::TextButton("Connect"));
    addAndMakeVisible(connectButton.get());
    connectButton->setButtonText("Connect");
    connectButton->addListener(this);

    // Cancel button
    cancelButton.reset(new juce::TextButton("Cancel"));
    addAndMakeVisible(cancelButton.get());
    cancelButton->setButtonText("Cancel");
    cancelButton->addListener(this);
}

PasswordInputOverlay::~PasswordInputOverlay()
{
    titleLabel = nullptr;
    ssidLabel = nullptr;
    passwordLabel = nullptr;
    passwordEditor = nullptr;
    connectButton = nullptr;
    cancelButton = nullptr;
}

void PasswordInputOverlay::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xc0000000));
}

void PasswordInputOverlay::resized()
{
    auto bounds = getLocalBounds();

    titleLabel->setBounds(16, 16, bounds.getWidth() - 32, 32);

    ssidLabel->setBounds(16, 56, bounds.getWidth() - 32, 24);

    passwordLabel->setBounds(16, 88, bounds.getWidth() - 32, 24);

    passwordEditor->setBounds(16, 120, bounds.getWidth() - 32, 24);

    connectButton->setBounds((bounds.getWidth() / 2) + 8, 160, 128, 24);

    cancelButton->setBounds((bounds.getWidth() / 2) - 136, 160, 128, 24);
}

void PasswordInputOverlay::buttonClicked(juce::Button* button)
{
    if (button == connectButton.get())
    {
        juce::String password = passwordEditor->getText();
        parentSelector->onPasswordEntered(password);
    }
    else if (button == cancelButton.get())
    {
        parentSelector->onPasswordCancelled();
    }
}

void PasswordInputOverlay::textEditorReturnKeyPressed(juce::TextEditor& editor)
{
    if (&editor == passwordEditor.get())
    {
        buttonClicked(connectButton.get());
    }
}

void PasswordInputOverlay::setSSID(const juce::String& ssid)
{
    ssidLabel->setText("SSID: " + ssid, juce::dontSendNotification);
}

void PasswordInputOverlay::grabPasswordFocus()
{
    passwordEditor->grabKeyboardFocus();
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="R2WiFiSelector" componentName=""
                 parentClasses="public juce::Component, private juce::Timer, juce::ListBoxModel"
                 constructorParams="std::function&lt;void()&gt; closeCallback"
                 variableInitialisers="onCloseCallback(closeCallback)" snapPixels="8"
                 snapActive="1" snapShown="1" overlayOpacity="0.330" fixedSize="1"
                 initialWidth="568" initialHeight="320">
  <BACKGROUND backgroundColour="ff323e44"/>
  <GENERICCOMPONENT name="" id="360ef4aca21e0008" memberName="listBox" virtualName="juce::ListBox"
                    explicitFocusOrder="0" pos="0Cc 40 32M 120M" class="juce::ListBox"
                    params="&quot;WiFiListBox&quot;, this"/>
  <LABEL name="" id="fad258a453d0f956" memberName="labelStatus" virtualName=""
         explicitFocusOrder="0" pos="0Cc 8Rr 32M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="(status)" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="" id="98d84e4143be47d3" memberName="buttonConnect" virtualName=""
              explicitFocusOrder="0" pos="0Cc 40Rr 152 24" buttonText="Connect"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="a53969a9a15d01fe" memberName="buttonClose" virtualName=""
              explicitFocusOrder="0" pos="16Rr 8 24 24" buttonText="X" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="3b484bcc80836c0d" memberName="labelTitle" virtualName=""
         explicitFocusOrder="0" pos="24 8 74M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Wi-Fi Connection" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
}   //  namespace r2juce
//[/EndFile]

