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

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
#include "Credentials.h"
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
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
    comboService->addItem (TRANS ("Google Drive"), 2);
    comboService->addItem (TRANS ("OneDrive"), 3);
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

    textButtonSignOut->setBounds (424, 16, 136, 24);

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
                                        TRANS ("status")));
    addAndMakeVisible (labelStatus.get());
    labelStatus->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelStatus->setJustificationType (juce::Justification::centredLeft);
    labelStatus->setEditable (false, false, false);
    labelStatus->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelStatus->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));


    //[UserPreSize]
    setWantsKeyboardFocus(true);

    // Manager setup
    cloudManager = std::make_unique<r2juce::R2CloudManager>();
    cloudManager->setGoogleCredentials(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET);
    cloudManager->setOneDriveCredentials(ONEDRIVE_CLIENT_ID, ONEDRIVE_CLIENT_SECRET);

    // Connect to the manager's state changes
    cloudManager->onStateChanged = [this](const r2juce::R2CloudManager::AppState& newState) {
        juce::MessageManager::callAsync([this, newState]() {
            updateUiForState(newState);
        });
    };

    // Initialize PropertiesFile for settings
    juce::File settingsDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                 .getChildFile ("CloudDoc");

    if (!settingsDir.exists())
        settingsDir.createDirectory();

    juce::File settingsFileLocation = settingsDir.getChildFile ("settings.xml");

    settingsFile.reset (new juce::PropertiesFile (settingsFileLocation,
                                                   juce::PropertiesFile::Options {}));

    // Set initial UI state from manager
    updateUiForState(cloudManager->getInitialState());

    // Defer loading of last session state until after the component is fully initialized
    juce::Timer::callAfterDelay(1, [this] {
        loadSettings();
    });

    // Initialize DropArea
    dropArea->onFileDropped = [this](const juce::String& filePath, const juce::MemoryBlock& fileContent) {
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
    saveSettings(); // Save settings on application exit
    if (cloudManager)
        cloudManager->onStateChanged = nullptr;
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

        switch (selectedId)
        {
            case 2: serviceToSelect = r2juce::R2CloudManager::ServiceType::GoogleDrive; break;
            case 3: serviceToSelect = r2juce::R2CloudManager::ServiceType::OneDrive; break;
            case 1:
            default: serviceToSelect = r2juce::R2CloudManager::ServiceType::Local; break;
        }

        cloudManager->userSelectedService(serviceToSelect);
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
        cloudManager->userSignedOut();
        //[/UserButtonCode_textButtonSignOut]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainComponent::updateUiForState(const r2juce::R2CloudManager::AppState& state)
{
    // Set ComboBox selection without triggering a callback loop
    int serviceId = 1;
    switch (state.selectedService)
    {
        case r2juce::R2CloudManager::ServiceType::GoogleDrive: serviceId = 2; break;
        case r2juce::R2CloudManager::ServiceType::OneDrive:   serviceId = 3; break;
        case r2juce::R2CloudManager::ServiceType::Local:
        default: serviceId = 1; break;
    }
    if (comboService->getSelectedId() != serviceId)
        comboService->setSelectedId(serviceId, juce::dontSendNotification);

    // Update the status label
    if (labelStatus)
    {
        labelStatus->setText(state.statusLabelText, juce::dontSendNotification);
        if (state.authStatus == r2juce::R2CloudManager::AuthStatus::Authenticated)
            labelStatus->setColour(juce::Label::textColourId, juce::Colours::lightgreen);
        else if (state.authStatus == r2juce::R2CloudManager::AuthStatus::Error)
            labelStatus->setColour(juce::Label::textColourId, juce::Colours::orangered);
        else
            labelStatus->setColour(juce::Label::textColourId, juce::Colours::white);
    }

    // Update button states
    textButtonSignOut->setEnabled(state.isSignOutButtonEnabled);
    textButtonLoad->setEnabled(state.areFileButtonsEnabled);
    textButtonSave->setEnabled(state.areFileButtonsEnabled);

    // Update ComboBox state
    comboService->setEnabled(state.isComboBoxEnabled);

    // Manage Auth UI visibility
    if (state.needsAuthUi && authComponent == nullptr)
    {
        showAuthUI();
    }
    else if (!state.needsAuthUi && authComponent != nullptr)
    {
        hideAuthUI();
    }
}

void MainComponent::showAuthUI()
{
    if (authComponent != nullptr) return;

    authComponent = std::make_unique<r2juce::R2CloudAuthComponent>();
    
    auto currentState = cloudManager->getCurrentState();
    if (currentState.selectedService == r2juce::R2CloudManager::ServiceType::GoogleDrive)
    {
        authComponent->setServiceType(r2juce::R2CloudAuthComponent::ServiceType::GoogleDrive);
        authComponent->setGoogleCredentials(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET);
    }
    else if (currentState.selectedService == r2juce::R2CloudManager::ServiceType::OneDrive)
    {
        authComponent->setServiceType(r2juce::R2CloudAuthComponent::ServiceType::OneDrive);
        authComponent->setOneDriveCredentials(ONEDRIVE_CLIENT_ID, ONEDRIVE_CLIENT_SECRET);
    }

    authComponent->onAuthenticationComplete = [this](bool success, const juce::String& msg, const juce::String& access, const juce::String& refresh) {
        cloudManager->authenticationFinished(success, msg, access, refresh);
    };

    authComponent->onAuthenticationCancelled = [this]() {
        cloudManager->userCancelledAuthentication();
    };

    addAndMakeVisible(*authComponent);
    authComponent->setBounds(getLocalBounds());
    authComponent->startAuthentication();
}

void MainComponent::hideAuthUI()
{
    authComponent.reset();
}


void MainComponent::loadFromFile()
{
    auto path = textEditorPath->getText().trim();
    auto filename = textEditorFilename->getText().trim();

    if (filename.isEmpty())
    {
        showMessage("Error", "Please enter a filename");
        return;
    }
    
    juce::String fullPath;
    if (path.isNotEmpty())
    {
        if (path.endsWith("/") || path.endsWith("\\"))
            fullPath = path + filename;
        else
            fullPath = path + "/" + filename;
    }
    else
    {
        fullPath = filename;
    }

    saveSettings();

    cloudManager->loadFile(fullPath, [this](bool success, juce::String content, juce::String errorMessage)
    {
        if (success)
        {
            textEditorData->setText(content);
            showMessage("Success", "File loaded successfully");
        }
        else
        {
            showMessage("Error", "Failed to load file: " + errorMessage);
        }
    });
}

void MainComponent::saveToFile()
{
    auto path = textEditorPath->getText().trim();
    auto filename = textEditorFilename->getText().trim();
    
    if (filename.isEmpty())
    {
        showMessage("Error", "Please enter a filename");
        return;
    }

    juce::String fullPath;
    if (path.isNotEmpty())
    {
        if (path.endsWith("/") || path.endsWith("\\"))
            fullPath = path + filename;
        else
            fullPath = path + "/" + filename;
    }
    else
    {
        fullPath = filename;
    }
    
    auto content = textEditorData->getText();
    saveSettings();

    juce::MemoryBlock data(content.toRawUTF8(), content.getNumBytesAsUTF8());

    cloudManager->saveFile(fullPath, data, [this](bool success, juce::String errorMessage)
    {
        if (success)
            showMessage("Success", "File saved successfully");
        else
            showMessage("Error", "File save failed: " + errorMessage);
    });
}

void MainComponent::loadSettings()
{
    if (settingsFile == nullptr) return;

    juce::String lastPath = settingsFile->getValue(lastFilePathKey, "");
    juce::String lastFilename = settingsFile->getValue(lastFileNameKey, "my_document.txt");
    int lastServiceId = settingsFile->getIntValue(lastSelectedServiceKey, 1);

    textEditorPath->setText(lastPath, juce::dontSendNotification);
    textEditorFilename->setText(lastFilename, juce::dontSendNotification);

    if (comboService->getSelectedId() != lastServiceId)
    {
        comboService->setSelectedId(lastServiceId, juce::sendNotification);
    }
}

void MainComponent::saveSettings()
{
    if (settingsFile != nullptr)
    {
        settingsFile->setValue(lastFilePathKey, textEditorPath->getText().trim());
        settingsFile->setValue(lastFileNameKey, textEditorFilename->getText().trim());
        settingsFile->setValue(lastSelectedServiceKey, comboService->getSelectedId());
        settingsFile->saveIfNeeded();
    }
}

void MainComponent::showMessage(const juce::String& title, const juce::String& message)
{
    // This is a simple implementation. For a better CX, a non-blocking
    // toast notification would be preferable to a modal dialog.
    r2juce::R2AlertComponent::forOK(this, title, message);
}

void MainComponent::handleFileDroppedInArea(const juce::String& filePath, const juce::MemoryBlock& fileContentData)
{
    juce::File droppedFile (filePath);

    if (droppedFile.isDirectory())
    {
        showMessage("Upload Error", "Folder uploads are not supported.");
        return;
    }

    const int maxFileSizeMB = 10;
    if (droppedFile.getSize() > maxFileSizeMB * 1024 * 1024)
    {
        showMessage("Upload Error", "File size exceeds " + juce::String(maxFileSizeMB) + "MB limit.");
        return;
    }

    textEditorFilename->setText(droppedFile.getFileName());

    juce::String uploadPath = textEditorPath->getText().trim();
    if (uploadPath.isEmpty())
        uploadPath = droppedFile.getFileName();
    else
    {
        if (!uploadPath.endsWith("/") || !uploadPath.endsWith("\\"))
            uploadPath << "/";
        uploadPath << droppedFile.getFileName();
    }

    saveSettings();

    cloudManager->saveFile(uploadPath, fileContentData,
                           [this, uploadPath](bool success, juce::String errorMessage)
    {
        if (success)
            showMessage("Success", "File uploaded successfully to: " + uploadPath);
        else
            showMessage("Error", "File upload failed: " + errorMessage);
    });
}

//==============================================================================
// DropArea implementations
bool MainComponent::DropArea::isInterestedInFileDrag(const juce::StringArray& files)
{
    return files.size() == 1 && juce::File(files[0]).exists();
}

void MainComponent::DropArea::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);
    isHighlighted = false;
    repaint();

    if (files.isEmpty())
        return;

    juce::File droppedFile (files[0]);
    juce::MemoryBlock fileContentData;

    if (droppedFile.existsAsFile())
        droppedFile.loadFileAsData(fileContentData);

    if (onFileDropped)
        onFileDropped(droppedFile.getFullPathName(), fileContentData);
}

void MainComponent::DropArea::fileDragEnter(const juce::StringArray&, int, int)
{
    isHighlighted = true;
    repaint();
}

void MainComponent::DropArea::fileDragMove(const juce::StringArray&, int, int) {}

void MainComponent::DropArea::fileDragExit(const juce::StringArray&)
{
    isHighlighted = false;
    repaint();
}

//[/MiscUserCode]

