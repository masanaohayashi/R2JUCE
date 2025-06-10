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
#include "PluginProcessor.h"
#include "R2JUCE/R2JUCE.h"
#include "MainComponent.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class ScreenKeyboardAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    //==============================================================================
    ScreenKeyboardAudioProcessorEditor (ScreenKeyboardAudioProcessor& p);
    ~ScreenKeyboardAudioProcessorEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    void mouseDown (const juce::MouseEvent &event) override { grabKeyboardFocus(); }
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    ScreenKeyboardAudioProcessor& audioProcessor;
    std::unique_ptr<MainComponent> mainComponent;
    //[/UserVariables]

    //==============================================================================


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScreenKeyboardAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

