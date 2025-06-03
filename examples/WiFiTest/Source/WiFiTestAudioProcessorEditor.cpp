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

#include "WiFiTestAudioProcessorEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
WiFiTestAudioProcessorEditor::WiFiTestAudioProcessorEditor (WiFiTestAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    wifiSelector.reset (new r2juce::R2WiFiSelector ([this](){ wifiSelector = nullptr; }));
    addAndMakeVisible (wifiSelector.get());


    //[UserPreSize]
    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

WiFiTestAudioProcessorEditor::~WiFiTestAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    wifiSelector = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void WiFiTestAudioProcessorEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void WiFiTestAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    wifiSelector->setBounds ((getWidth() / 2) - ((getWidth() - 0) / 2), (getHeight() / 2) - ((getHeight() - 0) / 2), getWidth() - 0, getHeight() - 0);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="WiFiTestAudioProcessorEditor"
                 componentName="" parentClasses="public juce::AudioProcessorEditor"
                 constructorParams="WiFiTestAudioProcessor&amp; p" variableInitialisers="AudioProcessorEditor (&amp;p), audioProcessor (p)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="568" initialHeight="320">
  <BACKGROUND backgroundColour="ff323e44"/>
  <GENERICCOMPONENT name="" id="5d6801b9ed44862a" memberName="wifiSelector" virtualName=""
                    explicitFocusOrder="0" pos="0Cc 0Cc 0M 0M" class="r2juce::R2WiFiSelector"
                    params="[this](){ wifiSelector = nullptr; }"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

