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
class JuceWidgets  : public juce::Component,
                     public juce::Slider::Listener
{
public:
    //==============================================================================
    JuceWidgets ();
    ~JuceWidgets() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void mouseDown (const juce::MouseEvent &event) override { grabKeyboardFocus(); }
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* sliderThatWasMoved) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Label> label;
    std::unique_ptr<juce::TextEditor> textEditor;
    std::unique_ptr<juce::Slider> slider;
    std::unique_ptr<juce::Slider> knob;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JuceWidgets)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

