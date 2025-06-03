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
#include <list>

namespace r2juce {

class R2ScreenKeyboard;
class R2ScreenKeyboardListener
{
public:
    virtual ~R2ScreenKeyboardListener() {}
    virtual void onSoftKeyboardKeyPressed (R2ScreenKeyboard* component, const juce::String& key) {}
};
//[/Headers]



//==============================================================================
/**
 //[Comments]
 An auto-generated component, created by the Projucer.
 
 Describe your class and how it works here!
 //[/Comments]
 */
class R2ScreenKeyboard  : public juce::Component,
public juce::Button::Listener
{
public:
    //==============================================================================
    R2ScreenKeyboard ();
    ~R2ScreenKeyboard() override;
    
    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void setTextEditor(juce::TextEditor* editor);
    void addListener (R2ScreenKeyboardListener* listener);
    void removeListener (R2ScreenKeyboardListener* listener);
    void setEnterCallback(std::function<void()> callback) { enterCallback = callback; }
    //[/UserMethods]
    
    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;
    
    
    
private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    void updateButtons();
    
    std::list<R2ScreenKeyboardListener*> listeners;
    juce::TextEditor* textEditor;
    bool shiftEnabled;
    bool numericEnabled;
    std::function<void()> enterCallback = nullptr;
    //[/UserVariables]
    
    //==============================================================================
    std::unique_ptr<juce::TextButton> buttonQ;
    std::unique_ptr<juce::TextButton> buttonW;
    std::unique_ptr<juce::TextButton> buttonE;
    std::unique_ptr<juce::TextButton> buttonR;
    std::unique_ptr<juce::TextButton> buttonT;
    std::unique_ptr<juce::TextButton> buttonY;
    std::unique_ptr<juce::TextButton> buttonU;
    std::unique_ptr<juce::TextButton> buttonI;
    std::unique_ptr<juce::TextButton> buttonO;
    std::unique_ptr<juce::TextButton> buttonP;
    std::unique_ptr<juce::TextButton> buttonA;
    std::unique_ptr<juce::TextButton> buttonS;
    std::unique_ptr<juce::TextButton> buttonD;
    std::unique_ptr<juce::TextButton> buttonF;
    std::unique_ptr<juce::TextButton> buttonG;
    std::unique_ptr<juce::TextButton> buttonH;
    std::unique_ptr<juce::TextButton> buttonJ;
    std::unique_ptr<juce::TextButton> buttonK;
    std::unique_ptr<juce::TextButton> buttonL;
    std::unique_ptr<juce::TextButton> buttonShift;
    std::unique_ptr<juce::TextButton> buttonZ;
    std::unique_ptr<juce::TextButton> buttonX;
    std::unique_ptr<juce::TextButton> buttonC;
    std::unique_ptr<juce::TextButton> buttonV;
    std::unique_ptr<juce::TextButton> buttonB;
    std::unique_ptr<juce::TextButton> buttonN;
    std::unique_ptr<juce::TextButton> buttonM;
    std::unique_ptr<juce::TextButton> buttonBackspace;
    std::unique_ptr<juce::TextButton> buttonNumeric;
    std::unique_ptr<juce::TextButton> buttonSpace;
    std::unique_ptr<juce::TextButton> buttonEnter;
    std::unique_ptr<juce::TextButton> buttonCursorLeft;
    std::unique_ptr<juce::TextButton> buttonCursorRight;
    
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2ScreenKeyboard)
};

//[EndFile] You can add extra defines here...
}   //  namespace r2juce
//[/EndFile]

