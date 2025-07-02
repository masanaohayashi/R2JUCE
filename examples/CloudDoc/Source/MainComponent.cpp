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


    //[UserPreSize]
    DBG("OneDrive ClientID: " + juce::String(ONEDRIVE_CLIENT_ID));
    DBG("OneDrive Secret: " + juce::String(ONEDRIVE_CLIENT_SECRET));

    cloudManager = std::make_unique<r2juce::R2CloudManager>();
    cloudManager->setGoogleCredentials(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET);
    cloudManager->setOneDriveCredentials(ONEDRIVE_CLIENT_ID, ONEDRIVE_CLIENT_SECRET);

    cloudManager->onAuthStatusChanged = [this](r2juce::R2CloudManager::AuthStatus status) {
        handleAuthStatusChanged(status);
    };

    cloudManager->onServiceChanged = [this](r2juce::R2CloudManager::ServiceType service) {
        handleServiceChanged(service);
    };

    // Initialize PropertiesFile for settings
    juce::File settingsDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                                 .getChildFile (JucePlugin_Manufacturer) // Use manufacturer name for top-level folder
                                 .getChildFile (JucePlugin_Name);        // Use product name for subfolder

    juce::File settingsFileLocation = settingsDir.getChildFile ("settings.xml");

    settingsFile.reset (new juce::PropertiesFile (settingsFileLocation,
                                                   juce::PropertiesFile::Options {}));

    // Load last saved settings
    int lastServiceId = settingsFile->getIntValue(lastSelectedServiceKey, 1); // Default to Local
    juce::String lastPath = settingsFile->getValue(lastFilePathKey, "");
    juce::String lastFilename = settingsFile->getValue(lastFileNameKey, "my_document.txt");

    // Apply loaded settings to UI
    textEditorPath->setText(lastPath, juce::dontSendNotification);
    textEditorFilename->setText(lastFilename, juce::dontSendNotification);
    comboService->setSelectedId(lastServiceId, juce::sendNotification); // This will trigger comboBoxChanged and select the service

    // Initialize DropArea without direct dependencies
    dropArea.reset (new DropArea());
    addAndMakeVisible (dropArea.get());
    dropArea->setBounds (472, 72, 86, 96);

    // Set the callback for when a file is dropped
    // Modified: fileContentをjuce::MemoryBlockで受け取るように変更
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
        auto selectedId = comboService->getSelectedId();

        switch (selectedId)
        {
            case 1: // Local Storage
                DBG("MainComponent::comboBoxChanged() - Selecting Local Storage");
                cloudManager->selectService(r2juce::R2CloudManager::ServiceType::Local);
                break;

            case 2: // Google Drive
                DBG("MainComponent::comboBoxChanged() - Selecting Google Drive");
                cloudManager->selectService(r2juce::R2CloudManager::ServiceType::GoogleDrive);
                if (cloudManager->needsAuthentication()) {
                    DBG("MainComponent::comboBoxChanged() - Google Drive needs authentication. Showing UI.");
                    cloudManager->showAuthenticationUI(this);
                }
                break;

            case 3: // OneDrive
                DBG("MainComponent::comboBoxChanged() - Selecting OneDrive");
                cloudManager->selectService(r2juce::R2CloudManager::ServiceType::OneDrive);
                if (cloudManager->needsAuthentication()) {
                    DBG("MainComponent::comboBoxChanged() - OneDrive needs authentication. Showing UI.");
                    cloudManager->showAuthenticationUI(this);
                }
                break;
        }
        // Save the selected service type
        if (settingsFile != nullptr)
            settingsFile->setValue(lastSelectedServiceKey, selectedId);
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
        DBG("MainComponent::buttonClicked() - Load button clicked.");
        loadFromFile();
        saveSettings(); // Save current path and filename on load
        //[/UserButtonCode_textButtonLoad]
    }
    else if (buttonThatWasClicked == textButtonSave.get())
    {
        //[UserButtonCode_textButtonSave] -- add your button handler code here..
        DBG("MainComponent::buttonClicked() - Save button clicked.");
        saveToFile();
        saveSettings(); // Save current path and filename on save
        //[/UserButtonCode_textButtonSave]
    }
    else if (buttonThatWasClicked == textButtonSignOut.get())
    {
        //[UserButtonCode_textButtonSignOut] -- add your button handler code here..
        DBG("MainComponent::buttonClicked() - Sign Out button clicked.");
        cloudManager->signOut();

        // After signing out, switch back to "Local only" service.
        // The ID for "Local only" is 1. Using sendNotification will trigger
        // the comboBoxChanged callback, which in turn updates the UI state,
        // including disabling the signout button.
        comboService->setSelectedId(1, juce::sendNotification);
        //[/UserButtonCode_textButtonSignOut]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainComponent::loadFromFile()
{
    auto filename = textEditorFilename->getText().trim();
    DBG("MainComponent::loadFromFile() - Filename: " + filename);

    if (filename.isEmpty())
    {
        showMessage("Error", "Please enter a filename");
        return;
    }

    // Note: loadFile still uses string content. If you plan to load binary, you might need another method.
    cloudManager->loadFile(filename, [this](bool success, juce::String content, juce::String errorMessage)
    {
        DBG(juce::String("MainComponent::loadFromFile() - Callback received. Success: ") + juce::String(success? "true": "false"));
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
    auto filePath = textEditorFilename->getText().trim();
    auto content = textEditorData->getText();
    DBG("MainComponent::saveToFile() - File path: " + filePath);

    if (filePath.isEmpty())
    {
        showMessage("Error", "Please enter a filename or path");
        return;
    }

    // Convert string content to MemoryBlock for saving
    juce::MemoryBlock data(content.toRawUTF8(), content.getNumBytesAsUTF8());

    // R2AlertComponentをローカルスコープで動的に生成し、ポインタで操作
    auto* currentSaveAlert = r2juce::R2AlertComponent::forProgress(this, "Saving File", "Uploading " + filePath + "...", -1.0,
        // Cancelボタンが押された場合のコールバック
        [](int buttonIndex) {
            if (buttonIndex == 1) // "Cancel"ボタンが1番目のボタンとして設定されている場合
            {
                DBG("File upload cancelled by user.");
                // 現状のR2CloudManagerにはキャンセル機構がないため、ここではダイアログを閉じるのみです。
            }
        });

    // safeAlertとしてポインタを保持し、ラムダキャプチャで使用
    juce::Component::SafePointer<r2juce::R2AlertComponent> safeSaveAlert = currentSaveAlert;

    cloudManager->saveFile(filePath, data, [this, safeSaveAlert](bool success, juce::String errorMessage)
    {
        // UIスレッドで処理するためにMessageManager::callAsyncを使用
        juce::MessageManager::callAsync([this, success, errorMessage, safeSaveAlert]() {
            if (safeSaveAlert != nullptr)
            {
                // ダイアログを閉じる
                safeSaveAlert->close();
            }

            // 元のshowMessageも必要に応じて残す
            if (success)
                showMessage("Success", "File saved successfully");
            else
                showMessage("Error", "File save failed: " + errorMessage);
        });
    });
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

void MainComponent::handleAuthStatusChanged(r2juce::R2CloudManager::AuthStatus status)
{
    // Use SafePointer to prevent crashes if component is deleted during async execution
    auto safeThis = juce::Component::SafePointer<MainComponent>(this);

    juce::MessageManager::callAsync([safeThis, status]()
    {
        // CRITICAL CHECK: Ensure component still exists before accessing
        if (safeThis == nullptr)
        {
            DBG("MainComponent was deleted, skipping auth status update");
            return;
        }

        switch (status)
        {
            case r2juce::R2CloudManager::AuthStatus::Authenticated:
                safeThis->showMessage("Success", "Successfully authenticated with cloud service");
                if (safeThis->textButtonSignOut != nullptr)
                    safeThis->textButtonSignOut->setEnabled(true);
                break;

            case r2juce::R2CloudManager::AuthStatus::NotAuthenticated:
                if (safeThis->textButtonSignOut != nullptr)
                    safeThis->textButtonSignOut->setEnabled(false);
                break;

            case r2juce::R2CloudManager::AuthStatus::Error:
                safeThis->showMessage("Error", "Authentication failed");
                if (safeThis->textButtonSignOut != nullptr)
                    safeThis->textButtonSignOut->setEnabled(false);
                break;

            case r2juce::R2CloudManager::AuthStatus::Authenticating:
                break;
        }
    });
}

void MainComponent::handleServiceChanged(r2juce::R2CloudManager::ServiceType service)
{
    // Use SafePointer to prevent crashes if component is deleted during async execution
    auto safeThis = juce::Component::SafePointer<MainComponent>(this);

    juce::MessageManager::callAsync([safeThis, service]()
    {
        // CRITICAL CHECK: Ensure component still exists before accessing
        if (safeThis == nullptr)
        {
            DBG("MainComponent was deleted, skipping service change update");
            return;
        }

        switch (service)
        {
            case r2juce::R2CloudManager::ServiceType::Local:
                if (safeThis->textButtonSignOut != nullptr)
                    safeThis->textButtonSignOut->setEnabled(false);
                break;

            case r2juce::R2CloudManager::ServiceType::GoogleDrive:
            case r2juce::R2CloudManager::ServiceType::OneDrive:
                if (safeThis->textButtonSignOut != nullptr && safeThis->cloudManager != nullptr)
                {
                    safeThis->textButtonSignOut->setEnabled(
                        safeThis->cloudManager->getAuthStatus() == r2juce::R2CloudManager::AuthStatus::Authenticated
                    );
                }
                break;
        }
    });
}

void MainComponent::showMessage(const juce::String& title, const juce::String& message)
{
    r2juce::R2AlertComponent::forOK(this, title, message);
}

// Modified: fileContentをjuce::MemoryBlockで受け取る
void MainComponent::handleFileDroppedInArea(const juce::String& filePath, const juce::MemoryBlock& fileContentData)
{
    juce::File droppedFile (filePath);

    // 1. Check if the dropped item is a folder
    if (droppedFile.isDirectory())
    {
        showMessage("Upload Error", "Folder uploads are not supported.");
        return;
    }

    // 2. Check file size (10MB limit)
    const int maxFileSizeMB = 10;
    const juce::int64 maxFileSizeBytes = static_cast<juce::int64>(maxFileSizeMB) * 1024 * 1024; // juce::int64 にキャスト

    if (droppedFile.getSize() > maxFileSizeBytes)
    {
        showMessage("Upload Error", "File size exceeds " + juce::String(maxFileSizeMB) + "MB limit.");
        return;
    }

    // 3. Update filename
    if (textEditorFilename != nullptr)
        textEditorFilename->setText(droppedFile.getFileName());

    // 4. Determine upload path
    juce::String uploadFileName = droppedFile.getFileName();
    juce::String uploadPath = textEditorPath->getText().trim();

    // Construct the full path for upload: path/filename
    if (uploadPath.isEmpty())
    {
        uploadPath = uploadFileName; // If path is empty, upload to root with just the filename
    }
    else
    {
        // Ensure path does not end with a slash if it's not root, then append filename
        if (!uploadPath.endsWith("/"))
            uploadPath << "/";
        uploadPath << uploadFileName;
    }

    // 5. Save the dropped file content (MemoryBlock) to the currently selected cloud service
    if (cloudManager != nullptr)
    {
        cloudManager->saveFile(uploadPath, fileContentData,
                               [this, uploadPath](bool success, juce::String errorMessage)
        {
            if (success)
                showMessage("Success", "File uploaded successfully to: " + uploadPath);
            else
                showMessage("Error", "File upload failed: " + errorMessage);
        });
    }
    saveSettings(); // Save current path and filename on file drop
}

//==============================================================================
// DropArea implementations
bool MainComponent::DropArea::isInterestedInFileDrag(const juce::StringArray& files)
{
    // Only interested if exactly one file is dragged and it exists
    return files.size() == 1 && juce::File(files[0]).exists(); // existsAsFile() is too restrictive for folder check
}

void MainComponent::DropArea::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);

    // Reset highlight when files are dropped
    isHighlighted = false;
    repaint();

    if (files.isEmpty())
        return;

    juce::File droppedFile (files[0]);
    juce::String filePath = droppedFile.getFullPathName();
    juce::MemoryBlock fileContentData; // MemoryBlock for file content

    // Read file content into MemoryBlock immediately in DropArea
    if (droppedFile.existsAsFile())
    {
        droppedFile.loadFileAsData(fileContentData);
    }
    else
    {
        // If it's a folder or doesn't exist as a file, let the handler check
        // We still pass the path to the handler for folder check
        DBG("Dropped item is not a file or does not exist: " + filePath);
    }

    // Call the callback to notify MainComponent
    if (onFileDropped)
    {
        onFileDropped(filePath, fileContentData); // MemoryBlockを渡す
    }
}

void MainComponent::DropArea::fileDragEnter(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(files, x, y);
    // Set highlight when files enter the area
    isHighlighted = true;
    repaint();
}

void MainComponent::DropArea::fileDragMove(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(files, x, y);
    // No change in highlight needed here, as it's already highlighted by fileDragEnter.
    // If you had more sophisticated highlight based on mouse position within the component, it would go here.
}

// あなたの指示に従い、`fileDragExit`を引数付きで実装します。
// ただし、これはJUCEの公式APIとは異なるため、コンパイルエラーが発生する可能性があります。
void MainComponent::DropArea::fileDragExit(const juce::StringArray& files)
{
    juce::ignoreUnused(files);
    // Clear highlight when files leave the area
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
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
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
            items="Local only&#10;Google Drive&#10;OneDrive" textWhenNonSelected=""
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
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

