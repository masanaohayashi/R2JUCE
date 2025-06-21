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
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainComponent  : public juce::Component,
                       public juce::ComboBox::Listener,
                       public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent ();
    ~MainComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    void setupUI();
    void loadFromFile();
    void saveToFile();
    void showMessage(const juce::String& title, const juce::String& message);

    // R2CloudManagerのコールバック
    void handleAuthStatusChanged(r2juce::R2CloudManager::AuthStatus status);
    void handleServiceChanged(r2juce::R2CloudManager::ServiceType service);

    // R2JUCE Cloud Manager（すべてのクラウド機能を管理）
    std::unique_ptr<r2juce::R2CloudManager> cloudManager;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Label> labelCloudService;
    std::unique_ptr<juce::ComboBox> comboService;
    std::unique_ptr<r2juce::R2TextEditor> textEditorData;
    std::unique_ptr<juce::TextButton> textButtonLoad;
    std::unique_ptr<juce::TextButton> textButtonSave;
    std::unique_ptr<juce::Label> labelFilename;
    std::unique_ptr<r2juce::R2TextEditor> textEditorFilename;
    std::unique_ptr<juce::TextButton> textButtonSignOut;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

