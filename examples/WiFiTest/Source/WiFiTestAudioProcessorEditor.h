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
#include "WiFiTestAudioProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class WiFiTestAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    //==============================================================================
    WiFiTestAudioProcessorEditor (WiFiTestAudioProcessor& p);
    ~WiFiTestAudioProcessorEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    WiFiTestAudioProcessor& audioProcessor;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<r2juce::R2WiFiSelector> wifiSelector;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WiFiTestAudioProcessorEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

