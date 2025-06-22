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
namespace r2juce {
//[/Headers]


//==============================================================================
/**
 //[Comments]
 An auto-generated component, created by the Projucer.
 
 Describe your class and how it works here!
 //[/Comments]
 */
class R2AlertComponent  : public juce::Component,
public juce::Button::Listener
{
    public:
    //==============================================================================
    R2AlertComponent (juce::Component* parent, const juce::String& title, const juce::String& message, const juce::StringArray& buttonLabels, std::function<void(int)> callback);
    ~R2AlertComponent() override;
    
    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    bool keyPressed (const juce::KeyPress& key) override;
    static void forOK (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback=nullptr);
    static void forYesNo (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback=nullptr);
    static void forYesNoCancel (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback=nullptr);
    //[/UserMethods]
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;
    
    
    
    private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    void buttonClicked (int index);
    void updateButtonFocus();
    
    juce::Component* parentComponent;
    std::function<void(int)> onResult;
    int numButtons;
    int selectedButtonIndex;
    //[/UserVariables]
    
    //==============================================================================
    std::unique_ptr<juce::Label> labelTitle;
    std::unique_ptr<juce::Label> labelMessage;
    std::unique_ptr<juce::TextButton> button1;
    std::unique_ptr<juce::TextButton> button2;
    std::unique_ptr<juce::TextButton> button3;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2AlertComponent)
};

//[EndFile] You can add extra defines here...
}   //  namespace r2juce
//[/EndFile]

