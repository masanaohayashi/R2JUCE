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
    comboService->addItem (TRANS ("Not use"), 1);
    comboService->addItem (TRANS ("Google Drive"), 2);
    comboService->addItem (TRANS ("OneDrive"), 3);
    comboService->addListener (this);

    comboService->setBounds (112, 16, 184, 24);

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

    labelFilename->setBounds (16, 48, 72, 24);

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

    textButtonSignOut->setBounds (320, 16, 136, 24);


    //[UserPreSize]
    DBG("OneDrive ClientID: " + juce::String(ONEDRIVE_CLIENT_ID));
    DBG("OneDrive Secret: " + juce::String(ONEDRIVE_CLIENT_SECRET));
    // R2CloudManagerの初期化
    cloudManager = std::make_unique<r2juce::R2CloudManager>();
    cloudManager->setGoogleCredentials(GOOGLE_CLIENT_ID, GOOGLE_CLIENT_SECRET);
    cloudManager->setOneDriveCredentials(ONEDRIVE_CLIENT_ID, ONEDRIVE_CLIENT_SECRET);

    // コールバック設定
    cloudManager->onAuthStatusChanged = [this](r2juce::R2CloudManager::AuthStatus status) {
        handleAuthStatusChanged(status);
    };

    cloudManager->onServiceChanged = [this](r2juce::R2CloudManager::ServiceType service) {
        handleServiceChanged(service);
    };

    // デフォルトでローカルを選択
    cloudManager->selectService(r2juce::R2CloudManager::ServiceType::Local);
    comboService->setSelectedId(1);
    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    labelCloudService = nullptr;
    comboService = nullptr;
    textEditorData = nullptr;
    textButtonLoad = nullptr;
    textButtonSave = nullptr;
    labelFilename = nullptr;
    textEditorFilename = nullptr;
    textButtonSignOut = nullptr;


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

    textEditorData->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 80, getWidth() - 32, 88);
    textButtonLoad->setBounds ((getWidth() / 2) + -80 - (136 / 2), 184, 136, 24);
    textButtonSave->setBounds ((getWidth() / 2) + 80 - (136 / 2), 184, 136, 24);
    textEditorFilename->setBounds (88, 48, getWidth() - 106, 24);
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
                cloudManager->selectService(r2juce::R2CloudManager::ServiceType::Local);
                break;

            case 2: // Google Drive
                cloudManager->selectService(r2juce::R2CloudManager::ServiceType::GoogleDrive);

                // 認証が必要な場合は自動で認証UIを表示
                if (cloudManager->needsAuthentication())
                {
                    cloudManager->showAuthenticationUI(this);
                }
                break;
            case 3: // OneDrive
                cloudManager->selectService(r2juce::R2CloudManager::ServiceType::OneDrive);
                // 認証が必要な場合は自動で認証UIを表示
                if (cloudManager->needsAuthentication())
                {
                    cloudManager->showAuthenticationUI(this);
                }
                break;
        }
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
        cloudManager->signOut();
        //[/UserButtonCode_textButtonSignOut]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void MainComponent::loadFromFile()
{
    auto filename = textEditorFilename->getText().trim();

    if (filename.isEmpty())
    {
        showMessage("Error", "Please enter a filename");
        return;
    }

    // R2CloudManagerを使ったシンプルなファイル読み込み
    cloudManager->loadFile(filename, [this](bool success, juce::String content, juce::String errorMessage)
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
    DBG("=== saveToFile() called ===");
    
    auto filename = textEditorFilename->getText().trim();
    auto content = textEditorData->getText();

    DBG("Filename: " + filename);
    DBG("Content length: " + juce::String(content.length()));
    
    if (filename.isEmpty())
    {
        DBG("Error: filename is empty");
        showMessage("Error", "Please enter a filename");
        return;
    }

    DBG("Current service type: " + juce::String((int)cloudManager->getCurrentService()));
    DBG("Auth status: " + juce::String((int)cloudManager->getAuthStatus()));

    // パス解析
    juce::String folderPath;
    juce::String actualFilename;
    
    if (filename.contains("/"))
    {
        // フォルダパス込みの場合
        auto lastSlash = filename.lastIndexOf("/");
        folderPath = filename.substring(0, lastSlash);  // "STUDIO-R/CloudDoc"
        actualFilename = filename.substring(lastSlash + 1);  // "test.txt"
        
        DBG("Folder path: " + folderPath);
        DBG("Actual filename: " + actualFilename);
        
        // フォルダパスでの保存
        cloudManager->saveFileWithPath(folderPath, actualFilename, content, [this](bool success, juce::String errorMessage)
        {
            if (success)
            {
                showMessage("Success", "File saved successfully");
            }
            else
            {
                showMessage("Error", "Failed to save file: " + errorMessage);
            }
        });
    }
    else
    {
        // 通常のルート保存
        cloudManager->saveFile(filename, content, [this](bool success, juce::String errorMessage)
        {
            if (success)
            {
                showMessage("Success", "File saved successfully");
            }
            else
            {
                showMessage("Error", "Failed to save file: " + errorMessage);
            }
        });
    }
}
void MainComponent::handleAuthStatusChanged(r2juce::R2CloudManager::AuthStatus status)
{
    switch (status)
    {
        case r2juce::R2CloudManager::AuthStatus::Authenticated:
            showMessage("Success", "Successfully authenticated with cloud service");
            textButtonSignOut->setEnabled(true);
            break;

        case r2juce::R2CloudManager::AuthStatus::NotAuthenticated:
            textButtonSignOut->setEnabled(false);
            break;

        case r2juce::R2CloudManager::AuthStatus::Error:
            showMessage("Error", "Authentication failed");
            textButtonSignOut->setEnabled(false);
            break;

        case r2juce::R2CloudManager::AuthStatus::Authenticating:
            // 認証中は特に何もしない（R2CloudAuthComponentが表示される）
            break;
    }
}

void MainComponent::handleServiceChanged(r2juce::R2CloudManager::ServiceType service)
{
    // サービス変更時の処理
    switch (service)
    {
        case r2juce::R2CloudManager::ServiceType::Local:
            textButtonSignOut->setEnabled(false);
            break;

        case r2juce::R2CloudManager::ServiceType::GoogleDrive:
            // 認証状態に応じてサインアウトボタンを有効/無効
            textButtonSignOut->setEnabled(
                cloudManager->getAuthStatus() == r2juce::R2CloudManager::AuthStatus::Authenticated
            );
            break;
        case r2juce::R2CloudManager::ServiceType::OneDrive:
            // 認証状態に応じてサインアウトボタンを有効/無効
            textButtonSignOut->setEnabled(
                cloudManager->getAuthStatus() == r2juce::R2CloudManager::AuthStatus::Authenticated
            );
            break;
    }
}

void MainComponent::showMessage(const juce::String& title, const juce::String& message)
{
    juce::AlertWindow::showAsync(
        juce::MessageBoxOptions()
            .withIconType(juce::MessageBoxIconType::InfoIcon)
            .withTitle(title)
            .withMessage(message)
            .withButton("OK"),
        nullptr
    );
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

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
            explicitFocusOrder="0" pos="112 16 184 24" editable="0" layout="33"
            items="Not use&#10;Google Drive&#10;OneDrive" textWhenNonSelected=""
            textWhenNoItems="(no choices)"/>
  <TEXTEDITOR name="" id="70766b9c8981f401" memberName="textEditorData" virtualName="r2juce::R2TextEditor"
              explicitFocusOrder="0" pos="0Cc 80 32M 88" bkgcol="ff000000"
              initialText="" multiline="1" retKeyStartsLine="1" readonly="0"
              scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="" id="353e971405dafbf9" memberName="textButtonLoad" virtualName=""
              explicitFocusOrder="0" pos="-80Cc 184 136 24" buttonText="Load from File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="bc50c6a3420f7701" memberName="textButtonSave" virtualName=""
              explicitFocusOrder="0" pos="80Cc 184 136 24" buttonText="Save to File"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="5000c983e2f78184" memberName="labelFilename" virtualName=""
         explicitFocusOrder="0" pos="16 48 72 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Filename" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="c69cf648ab860b39" memberName="textEditorFilename"
              virtualName="r2juce::R2TextEditor" explicitFocusOrder="0" pos="88 48 106M 24"
              bkgcol="ff000000" initialText="" multiline="0" retKeyStartsLine="0"
              readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <TEXTBUTTON name="" id="2c94a8614918cc8e" memberName="textButtonSignOut"
              virtualName="" explicitFocusOrder="0" pos="320 16 136 24" buttonText="Signout"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

