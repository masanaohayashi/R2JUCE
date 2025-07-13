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
#include "CloudDocAudioProcessor.h"
#include "Credentials.h"
//[/Headers]

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent (CloudDocAudioProcessor& p)
    : audioProcessor(p), cloudManager(p.getCloudManager())
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    labelCloudService.reset (new juce::Label (juce::String(),
                                              TRANS ("Cloud Service")));
    addAndMakeVisible (labelCloudService.get());
    labelCloudService->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelCloudService->setJustificationType (juce::Justification::centredLeft);
    labelCloudService->setEditable (false, false, false);
    labelCloudService->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelCloudService->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelCloudService->setBounds (16, 16, 96, 24);

    comboService.reset (new juce::ComboBox (juce::String()));
    addAndMakeVisible (comboService.get());
    comboService->setEditableText (false);
    comboService->setJustificationType (juce::Justification::centredLeft);
    comboService->setTextWhenNothingSelected (juce::String());
    comboService->setTextWhenNoChoicesAvailable (TRANS ("(no choices)"));
    comboService->addItem (TRANS ("Local only"), 1);
    comboService->addItem (TRANS ("iCloud"), 2);
    comboService->addItem (TRANS ("Google Drive"), 3);
    comboService->addItem (TRANS ("OneDrive"), 4);
    comboService->addListener (this);

    comboService->setBounds (112, 16, 200, 24);

    textEditorData.reset (new r2juce::R2TextEditor (juce::String()));
    addAndMakeVisible (textEditorData.get());
    textEditorData->setMultiLine (true);
    textEditorData->setReturnKeyStartsNewLine (true);
    textEditorData->setReadOnly (false);
    textEditorData->setScrollbarsShown (true);
    textEditorData->setCaretVisible (true);
    textEditorData->setPopupMenuEnabled (true);
    textEditorData->setColour (juce::TextEditor::backgroundColourId, juce::Colours::black);
    textEditorData->setText (juce::String());

    textButtonLoad.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (textButtonLoad.get());
    textButtonLoad->setButtonText (TRANS ("Load from File"));
    textButtonLoad->addListener (this);

    textButtonSave.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (textButtonSave.get());
    textButtonSave->setButtonText (TRANS ("Save to File"));
    textButtonSave->addListener (this);

    labelFilename.reset (new juce::Label (juce::String(),
                                          TRANS ("Filename")));
    addAndMakeVisible (labelFilename.get());
    labelFilename->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelFilename->setJustificationType (juce::Justification::centredLeft);
    labelFilename->setEditable (false, false, false);
    labelFilename->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelFilename->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelFilename->setBounds (16, 80, 72, 24);

    textEditorFilename.reset (new r2juce::R2TextEditor (juce::String()));
    addAndMakeVisible (textEditorFilename.get());
    textEditorFilename->setMultiLine (false);
    textEditorFilename->setReturnKeyStartsNewLine (false);
    textEditorFilename->setReadOnly (false);
    textEditorFilename->setScrollbarsShown (true);
    textEditorFilename->setCaretVisible (true);
    textEditorFilename->setPopupMenuEnabled (true);
    textEditorFilename->setColour (juce::TextEditor::backgroundColourId, juce::Colours::black);
    textEditorFilename->setText (juce::String());

    textButtonSignOut.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (textButtonSignOut.get());
    textButtonSignOut->setButtonText (TRANS ("Signout"));
    textButtonSignOut->addListener (this);

    textButtonSignOut->setBounds (328, 16, 136, 24);

    labelPath.reset (new juce::Label (juce::String(),
                                      TRANS ("Path")));
    addAndMakeVisible (labelPath.get());
    labelPath->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelPath->setJustificationType (juce::Justification::centredLeft);
    labelPath->setEditable (false, false, false);
    labelPath->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelPath->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelPath->setBounds (16, 48, 72, 24);

    textEditorPath.reset (new r2juce::R2TextEditor (juce::String()));
    addAndMakeVisible (textEditorPath.get());
    textEditorPath->setMultiLine (false);
    textEditorPath->setReturnKeyStartsNewLine (false);
    textEditorPath->setReadOnly (false);
    textEditorPath->setScrollbarsShown (true);
    textEditorPath->setCaretVisible (true);
    textEditorPath->setPopupMenuEnabled (true);
    textEditorPath->setColour (juce::TextEditor::backgroundColourId, juce::Colours::black);
    textEditorPath->setText (juce::String());

    labelData.reset (new juce::Label (juce::String(),
                                      TRANS ("Data")));
    addAndMakeVisible (labelData.get());
    labelData->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelData->setJustificationType (juce::Justification::centredLeft);
    labelData->setEditable (false, false, false);
    labelData->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelData->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelData->setBounds (16, 112, 72, 24);

    dropArea.reset (new DropArea());
    addAndMakeVisible (dropArea.get());

    dropArea->setBounds (472, 72, 86, 96);

    labelStatus.reset (new juce::Label (juce::String(),
                                        TRANS ("Cloud Service")));
    addAndMakeVisible (labelStatus.get());
    labelStatus->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelStatus->setJustificationType (juce::Justification::centredLeft);
    labelStatus->setEditable (false, false, false);
    labelStatus->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelStatus->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));


    //[UserPreSize]
#if !(JUCE_MAC || JUCE_IOS)
    // On non-Apple platforms, disable iCloud option.
    comboService->setItemEnabled(2, false);
#endif

#if JucePlugin_Build_AUv3 && !JUCE_STANDALONE_APPLICATION
    // In an AUv3 plugin, only Local and iCloud are reliable.
    // Disable other options that require unrestricted network access.
    comboService->setItemEnabled(3, false);  // Disable Google Drive
    comboService->setItemEnabled(4, false);  // Disable OneDrive
#endif

    setWantsKeyboardFocus(true);

    // Connect to the manager's state changes
    cloudManager.onStateChanged =
        [this](const r2juce::R2CloudManager::AppState& newState) {
        juce::MessageManager::callAsync(
            [this, newState]() { updateUiForState(newState); });
    };

    // Set initial UI state from processor's settings
    textEditorPath->setText(audioProcessor.getInitialPath(),
                            juce::dontSendNotification);
    textEditorFilename->setText(audioProcessor.getInitialFilename(),
                                juce::dontSendNotification);
    comboService->setSelectedId(audioProcessor.getInitialServiceId(),
                                juce::dontSendNotification);

    // Set initial UI state from manager, which will trigger the initial load
    updateUiForState(cloudManager.getInitialState());

    // Initialize DropArea
    dropArea->onFileDropped = [this](const juce::String& filePath,
                                     const juce::MemoryBlock& fileContent) {
        handleFileDroppedInArea(filePath, fileContent);
    };

    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    if (progressAlert != nullptr) {
        progressAlert->close();
        progressAlert = nullptr;
    }
    // Save current UI state back to the processor before closing.
    audioProcessor.setCurrentPath(textEditorPath->getText().trim());
    audioProcessor.setCurrentFilename(textEditorFilename->getText().trim());
    audioProcessor.setCurrentServiceId(comboService->getSelectedId());
    audioProcessor.setCurrentFileContent(textEditorData->getText()); // THIS LINE IS THE FIX

    cloudManager.onStateChanged = nullptr;
    //[/Destructor_pre]

    labelCloudService = nullptr;
    comboService = nullptr;
    textEditorData = nullptr;
    textButtonLoad = nullptr;
    textButtonSave = nullptr;
    labelFilename = nullptr;
    textEditorFilename = nullptr;
    textButtonSignOut = nullptr;
    labelPath = nullptr;
    textEditorPath = nullptr;
    labelData = nullptr;
    dropArea = nullptr;
    labelStatus = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    textEditorData->setBounds ((getWidth() / 2) + -8 - ((getWidth() - 192) / 2), 112, getWidth() - 192, 56);
    textButtonLoad->setBounds ((getWidth() / 2) + -80 - (136 / 2), 184, 136, 24);
    textButtonSave->setBounds ((getWidth() / 2) + 80 - (136 / 2), 184, 136, 24);
    textEditorFilename->setBounds (88, 80, getWidth() - 192, 24);
    textEditorPath->setBounds (88, 48, getWidth() - 192, 24);
    labelStatus->setBounds (8, getHeight() - 24, getWidth() - 16, 24);
    //[UserResized] Add your own custom resize handling here..
    if (authComponent != nullptr) authComponent->setBounds(getLocalBounds());
    //[/UserResized]
}

void MainComponent::comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged)
{
    //[UsercomboBoxChanged_Pre]
    //[/UsercomboBoxChanged_Pre]

    if (comboBoxThatHasChanged == comboService.get())
    {
        //[UserComboBoxCode_comboService] -- add your combo box handling code here..
        // This function now only tells the manager what the user wants.
        auto selectedId = comboService->getSelectedId();
        r2juce::R2CloudManager::ServiceType serviceToSelect;

        switch (selectedId) {
            case 1:
                serviceToSelect = r2juce::R2CloudManager::ServiceType::Local;
                break;
            case 2:
                serviceToSelect = r2juce::R2CloudManager::ServiceType::iCloudDrive;
                break;
            case 3:
                serviceToSelect = r2juce::R2CloudManager::ServiceType::GoogleDrive;
                break;
            case 4:
                serviceToSelect = r2juce::R2CloudManager::ServiceType::OneDrive;
                break;
            default:
                serviceToSelect = r2juce::R2CloudManager::ServiceType::Local;
                break;
        }

        cloudManager.userSelectedService(serviceToSelect);
        //[/UserComboBoxCode_comboService]
    }

    //[UsercomboBoxChanged_Post]
    //[/UsercomboBoxChanged_Post]
}

void MainComponent::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == textButtonLoad.get())
    {
        //[UserButtonCode_textButtonLoad] -- add your button handler code here..
        loadFromFile();
        //[/UserButtonCode_textButtonLoad]
    }
    else if (buttonThatWasClicked == textButtonSave.get())
    {
        //[UserButtonCode_textButtonSave] -- add your button handler code here..
        saveToFile();
        //[/UserButtonCode_textButtonSave]
    }
    else if (buttonThatWasClicked == textButtonSignOut.get())
    {
        //[UserButtonCode_textButtonSignOut] -- add your button handler code here..
        cloudManager.userSignedOut();
        //[/UserButtonCode_textButtonSignOut]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
// other code here...
void MainComponent::updateUiForState(
    const r2juce::R2CloudManager::AppState& state) {
    // Set ComboBox selection without triggering a callback loop
    int serviceId = 1;
    switch (state.selectedService) {
        case r2juce::R2CloudManager::ServiceType::Local:
            serviceId = 1;
            break;
        case r2juce::R2CloudManager::ServiceType::iCloudDrive:
            serviceId = 2;
            break;
        case r2juce::R2CloudManager::ServiceType::GoogleDrive:
            serviceId = 3;
            break;
        case r2juce::R2CloudManager::ServiceType::OneDrive:
            serviceId = 4;
            break;
    }
    if (comboService->getSelectedId() != serviceId)
        comboService->setSelectedId(serviceId, juce::dontSendNotification);

    // Update the status label
    if (labelStatus) {
        labelStatus->setText(state.statusLabelText, juce::dontSendNotification);
        if (state.authStatus == r2juce::R2CloudManager::AuthStatus::Authenticated)
            labelStatus->setColour(juce::Label::textColourId,
                                   juce::Colours::lightgreen);
        else if (state.authStatus == r2juce::R2CloudManager::AuthStatus::Error)
            labelStatus->setColour(juce::Label::textColourId,
                                   juce::Colours::orangered);
        else
            labelStatus->setColour(juce::Label::textColourId, juce::Colours::white);
    }

    // Update button states
    const bool isOperationInProgress = (progressAlert != nullptr);
    textButtonSignOut->setEnabled(state.isSignOutButtonEnabled &&
                                  !isOperationInProgress);
    textButtonLoad->setEnabled(state.areFileButtonsEnabled &&
                               !isOperationInProgress);
    textButtonSave->setEnabled(state.areFileButtonsEnabled &&
                               !isOperationInProgress);

    // Update ComboBox state
    comboService->setEnabled(state.isComboBoxEnabled && !isOperationInProgress);

    // Manage Auth UI visibility
    if (state.needsAuthUi && authComponent == nullptr) {
        showAuthUI();
    } else if (!state.needsAuthUi && authComponent != nullptr) {
        hideAuthUI();
    }

    // Trigger initial file load when service becomes authenticated
    if (state.authStatus == r2juce::R2CloudManager::AuthStatus::Authenticated &&
        !initialLoadAttempted) {
        initialLoadAttempted = true;
        loadFromFile(false);  // Load file content without showing messages
    }
}

void MainComponent::showAuthUI() {
    if (authComponent != nullptr) return;

    authComponent = std::make_unique<r2juce::R2CloudAuthComponent>();

    auto currentState = cloudManager.getCurrentState();
    if (currentState.selectedService ==
        r2juce::R2CloudManager::ServiceType::GoogleDrive) {
        authComponent->setServiceType(
            r2juce::R2CloudAuthComponent::ServiceType::GoogleDrive);
        authComponent->setGoogleCredentials(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET);
    } else if (currentState.selectedService ==
               r2juce::R2CloudManager::ServiceType::OneDrive) {
        authComponent->setServiceType(
            r2juce::R2CloudAuthComponent::ServiceType::OneDrive);
        authComponent->setOneDriveCredentials(ONEDRIVE_CLIENT_ID,
                                            ONEDRIVE_CLIENT_SECRET);
    }

    authComponent->onAuthenticationComplete =
        [this](bool success, const juce::String& msg, const juce::String& access,
               const juce::String& refresh) {
        cloudManager.authenticationFinished(success, msg, access, refresh);
    };

    authComponent->onAuthenticationCancelled = [this]() {
        cloudManager.userCancelledAuthentication();
    };

    addAndMakeVisible(*authComponent);
    authComponent->setBounds(getLocalBounds());
    authComponent->startAuthentication();
}

void MainComponent::hideAuthUI() { authComponent.reset(); }

void MainComponent::loadFromFile(bool showMessages) {
    if (progressAlert != nullptr && showMessages) return;

    auto path = textEditorPath->getText().trim();
    auto filename = textEditorFilename->getText().trim();

    if (filename.isEmpty()) {
        if (showMessages) showMessage("Error", "Please enter a filename");
        return;
    }

    juce::String fullPath;
    if (path.isNotEmpty()) {
        if (path.endsWith("/") || path.endsWith("\\"))
            fullPath = path + filename;
        else
            fullPath = path + "/" + filename;
    } else {
        fullPath = filename;
    }

    if (showMessages) {
        progressAlert = r2juce::R2AlertComponent::forProgress(
            this, "Loading File", "Loading '" + filename + "'...",
            -1.0,  // Indeterminate progress
            [this](int buttonIndex) {
            if (buttonIndex == 1)  // Cancel
            {
                // NOTE: Cloud operation cancellation is not implemented in this
                // version. The dialog will close itself. We just nullify the
                // pointer.
                progressAlert = nullptr;
                updateUiForState(cloudManager.getCurrentState());
            }
        });
        updateUiForState(cloudManager.getCurrentState());
    }

    cloudManager.loadFile(
        fullPath, [this, showMessages](bool success, juce::String content,
                                       juce::String errorMessage) {
        DBG("--- File Load Callback ---");
        DBG("Success: " << (success ? "true" : "false"));
        DBG("Content: " << content);
        DBG("Error Message: " << errorMessage);

        if (progressAlert != nullptr) {
            progressAlert->close();
            progressAlert = nullptr;
        }

        if (success) {
            textEditorData->setText(content);
            if (showMessages) showMessage("Success", "File loaded successfully");
        } else {
            if (showMessages)
                showMessage("Error", "Failed to load file: " + errorMessage);
        }

        if (showMessages) updateUiForState(cloudManager.getCurrentState());
    });
}

void MainComponent::saveToFile() {
    if (progressAlert != nullptr) return;

    auto path = textEditorPath->getText().trim();
    auto filename = textEditorFilename->getText().trim();

    if (filename.isEmpty()) {
        showMessage("Error", "Please enter a filename");
        return;
    }

    juce::String fullPath;
    if (path.isNotEmpty()) {
        if (path.endsWith("/") || path.endsWith("\\"))
            fullPath = path + filename;
        else
            fullPath = path + "/" + filename;
    } else {
        fullPath = filename;
    }

    auto content = textEditorData->getText();

    juce::MemoryBlock data(content.toRawUTF8(), content.getNumBytesAsUTF8());

    progressAlert = r2juce::R2AlertComponent::forProgress(
        this, "Saving File", "Saving '" + filename + "'...",
        -1.0,  // Indeterminate progress
        [this](int buttonIndex) {
        if (buttonIndex == 1)  // Cancel
        {
            progressAlert = nullptr;
            updateUiForState(cloudManager.getCurrentState());
        }
    });

    updateUiForState(cloudManager.getCurrentState());

    cloudManager.saveFile(
        fullPath, data, [this, filename](bool success, juce::String errorMessage) {
        if (progressAlert != nullptr) {
            progressAlert->close();
            progressAlert = nullptr;
        }

        if (success)
            showMessage("Success", "File saved successfully");
        else
            showMessage("Error", "File save failed: " + errorMessage);

        updateUiForState(cloudManager.getCurrentState());
    });
}

void MainComponent::showMessage(const juce::String& title,
                                const juce::String& message) {
    // This is a simple implementation. For a better CX, a non-blocking
    // toast notification would be preferable to a modal dialog.
    r2juce::R2AlertComponent::forOK(this, title, message);
}

void MainComponent::handleFileDroppedInArea(
    const juce::String& filePath, const juce::MemoryBlock& fileContentData) {
    if (progressAlert != nullptr) return;

    juce::File droppedFile(filePath);

    if (droppedFile.isDirectory()) {
        showMessage("Upload Error", "Folder uploads are not supported.");
        return;
    }

    const int maxFileSizeMB = 10;
    if (droppedFile.getSize() > maxFileSizeMB * 1024 * 1024) {
        showMessage("Upload Error",
                    "File size exceeds " + juce::String(maxFileSizeMB) +
                    "MB limit.");
        return;
    }

    juce::String uploadPath = textEditorPath->getText().trim();
    if (uploadPath.isEmpty())
        uploadPath = droppedFile.getFileName();
    else {
        if (!uploadPath.endsWith("/") && !uploadPath.endsWith("\\")) uploadPath << "/";
        uploadPath << droppedFile.getFileName();
    }

    // Save current settings (like path) before uploading
    audioProcessor.setCurrentPath(textEditorPath->getText().trim());
    audioProcessor.setCurrentFilename(textEditorFilename->getText().trim());
    audioProcessor.setCurrentServiceId(comboService->getSelectedId());

    progressAlert = r2juce::R2AlertComponent::forProgress(
        this, "Uploading File", "Uploading '" + droppedFile.getFileName() + "'...",
        -1.0,  // Indeterminate progress
        [this](int buttonIndex) {
        if (buttonIndex == 1)  // Cancel
        {
            progressAlert = nullptr;
            updateUiForState(cloudManager.getCurrentState());
        }
    });

    updateUiForState(cloudManager.getCurrentState());

    cloudManager.saveFile(
        uploadPath, fileContentData,
        [this, uploadPath](bool success, juce::String errorMessage) {
        if (progressAlert != nullptr) {
            progressAlert->close();
            progressAlert = nullptr;
        }

        if (success)
            showMessage("Success",
                        "File uploaded successfully to: " + uploadPath);
        else
            showMessage("Error", "File upload failed: " + errorMessage);

        updateUiForState(cloudManager.getCurrentState());
    });
}

//==============================================================================
// DropArea implementations
bool MainComponent::DropArea::isInterestedInFileDrag(
    const juce::StringArray& files) {
    return files.size() == 1 && juce::File(files[0]).exists();
}

void MainComponent::DropArea::filesDropped(const juce::StringArray& files,
                                           int x, int y) {
    juce::ignoreUnused(x, y);
    isHighlighted = false;
    repaint();

    if (files.isEmpty()) return;

    juce::File droppedFile(files[0]);
    juce::MemoryBlock fileContentData;

    if (droppedFile.existsAsFile())
        droppedFile.loadFileAsData(fileContentData);

    if (onFileDropped)
        onFileDropped(droppedFile.getFullPathName(), fileContentData);
}

void MainComponent::DropArea::fileDragEnter(const juce::StringArray&, int,
                                            int) {
    isHighlighted = true;
    repaint();
}

void MainComponent::DropArea::fileDragMove(const juce::StringArray&, int, int) {}

void MainComponent::DropArea::fileDragExit(const juce::StringArray&) {
    isHighlighted = false;
    repaint();
}

//[/MiscUserCode]


//==============================================================================
#if 0
/* -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public juce::Component" constructorParams="CloudDocAudioProcessor&amp; p"
                 variableInitialisers="audioProcessor(p), cloudManager(p.getCloudManager())"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="568" initialHeight="320">
  <BACKGROUND backgroundColour="ff323e44"/>
  <LABEL name="" id="2f02bb18c4366b87" memberName="labelCloudService"
         virtualName="" explicitFocusOrder="0" pos="16 16 96 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Cloud Service" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <COMBOBOX name="" id="1901ecb05f23698f" memberName="comboService" virtualName=""
            explicitFocusOrder="0" pos="112 16 200 24" editable="0" layout="33"
            items="Local only&#10;iCloud&#10;Google Drive&#10;OneDrive" textWhenNonSelected=""
            textWhenNoItems="(no choices)"/>
  <TEXTEDITOR name="" id="70766b9c8981f401" memberName="textEditorData" virtualName="r2juce::R2TextEditor"
              explicitFocusOrder="0" pos="-8Cc 112 192M 56" bkgcol="ff000000"
              initialText="" multiline="1" retKeyStartsLine="1" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="" id="353e971405dafbf9" memberName="textButtonLoad" virtualName=""
              explicitFocusOrder="0" pos="-80Cc 184 136 24" buttonText="Load from File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="bc50c6a3420f7701" memberName="textButtonSave" virtualName=""
              explicitFocusOrder="0" pos="80Cc 184 136 24" buttonText="Save to File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="5000c983e2f78184" memberName="labelFilename" virtualName=""
         explicitFocusOrder="0" pos="16 80 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Filename" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="c69cf648ab860b39" memberName="textEditorFilename"
              virtualName="r2juce::R2TextEditor" explicitFocusOrder="0" pos="88 80 192M 24"
              bkgcol="ff000000" initialText="" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="" id="2c94a8614918cc8e" memberName="textButtonSignOut"
              virtualName="" explicitFocusOrder="0" pos="328 16 136 24" buttonText="Signout"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="1a0e541e78f38ab1" memberName="labelPath" virtualName=""
         explicitFocusOrder="0" pos="16 48 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Path" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="5ca9d1288c8108b4" memberName="textEditorPath" virtualName="r2juce::R2TextEditor"
              explicitFocusOrder="0" pos="88 48 192M 24" bkgcol="ff000000"
              initialText="" multiline="0" retKeyStartsLine="0" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="" id="213fc5548588195d" memberName="labelData" virtualName=""
         explicitFocusOrder="0" pos="16 112 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Data" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <GENERICCOMPONENT name="" id="fd7275705d501103" memberName="dropArea" virtualName=""
                    explicitFocusOrder="0" pos="472 72 86 96" class="DropArea" params=""/>
  <LABEL name="" id="4021c25b87690560" memberName="labelStatus" virtualName=""
         explicitFocusOrder="0" pos="8 0Rr 16M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Cloud Service" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

