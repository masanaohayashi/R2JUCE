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

#include "AudioConfigEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
AudioConfigEditor::AudioConfigEditor (AudioConfigAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), configManager(AudioConfig")
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    labelOutputDevice.reset (new juce::Label (juce::String(),
                                              TRANS ("(placeholder)")));
    addAndMakeVisible (labelOutputDevice.get());
    labelOutputDevice->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelOutputDevice->setJustificationType (juce::Justification::centredLeft);
    labelOutputDevice->setEditable (false, false, false);
    labelOutputDevice->setColour (juce::Label::backgroundColourId, juce::Colours::black);
    labelOutputDevice->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelOutputDevice->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelOutputDevice->setBounds (16, 40, 568, 24);

    labelOutputDeviceCaption.reset (new juce::Label (juce::String(),
                                                     TRANS ("Output Device")));
    addAndMakeVisible (labelOutputDeviceCaption.get());
    labelOutputDeviceCaption->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelOutputDeviceCaption->setJustificationType (juce::Justification::centredLeft);
    labelOutputDeviceCaption->setEditable (false, false, false);
    labelOutputDeviceCaption->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelOutputDeviceCaption->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelOutputDeviceCaption->setBounds (16, 16, 568, 24);

    buttonRefresh.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonRefresh.get());
    buttonRefresh->setButtonText (TRANS ("Refresh"));
    buttonRefresh->addListener (this);

    labelInputDevice.reset (new juce::Label (juce::String(),
                                             TRANS ("(placeholder)")));
    addAndMakeVisible (labelInputDevice.get());
    labelInputDevice->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelInputDevice->setJustificationType (juce::Justification::centredLeft);
    labelInputDevice->setEditable (false, false, false);
    labelInputDevice->setColour (juce::Label::backgroundColourId, juce::Colours::black);
    labelInputDevice->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelInputDevice->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelInputDevice->setBounds (16, 104, 568, 24);

    labelInputDeviceCaption.reset (new juce::Label (juce::String(),
                                                    TRANS ("Input Device")));
    addAndMakeVisible (labelInputDeviceCaption.get());
    labelInputDeviceCaption->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelInputDeviceCaption->setJustificationType (juce::Justification::centredLeft);
    labelInputDeviceCaption->setEditable (false, false, false);
    labelInputDeviceCaption->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelInputDeviceCaption->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelInputDeviceCaption->setBounds (16, 80, 568, 24);

    labelBufferSize.reset (new juce::Label (juce::String(),
                                            TRANS ("(placeholder)")));
    addAndMakeVisible (labelBufferSize.get());
    labelBufferSize->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelBufferSize->setJustificationType (juce::Justification::centredLeft);
    labelBufferSize->setEditable (false, false, false);
    labelBufferSize->setColour (juce::Label::backgroundColourId, juce::Colours::black);
    labelBufferSize->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelBufferSize->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelBufferSize->setBounds (312, 168, 272, 16);

    labelBufferSizeCaption.reset (new juce::Label (juce::String(),
                                                   TRANS ("Buffer Size\n")));
    addAndMakeVisible (labelBufferSizeCaption.get());
    labelBufferSizeCaption->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelBufferSizeCaption->setJustificationType (juce::Justification::centredLeft);
    labelBufferSizeCaption->setEditable (false, false, false);
    labelBufferSizeCaption->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelBufferSizeCaption->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelBufferSizeCaption->setBounds (312, 144, 272, 16);

    labelSampleRate.reset (new juce::Label (juce::String(),
                                            TRANS ("(placeholder)")));
    addAndMakeVisible (labelSampleRate.get());
    labelSampleRate->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelSampleRate->setJustificationType (juce::Justification::centredLeft);
    labelSampleRate->setEditable (false, false, false);
    labelSampleRate->setColour (juce::Label::backgroundColourId, juce::Colours::black);
    labelSampleRate->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelSampleRate->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelSampleRate->setBounds (16, 168, 272, 16);

    labelSampleRateCaption.reset (new juce::Label (juce::String(),
                                                   TRANS ("Buffer Size\n")));
    addAndMakeVisible (labelSampleRateCaption.get());
    labelSampleRateCaption->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelSampleRateCaption->setJustificationType (juce::Justification::centredLeft);
    labelSampleRateCaption->setEditable (false, false, false);
    labelSampleRateCaption->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelSampleRateCaption->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelSampleRateCaption->setBounds (16, 144, 272, 16);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (600, 400);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

AudioConfigEditor::~AudioConfigEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    labelOutputDevice = nullptr;
    labelOutputDeviceCaption = nullptr;
    buttonRefresh = nullptr;
    labelInputDevice = nullptr;
    labelInputDeviceCaption = nullptr;
    labelBufferSize = nullptr;
    labelBufferSizeCaption = nullptr;
    labelSampleRate = nullptr;
    labelSampleRateCaption = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void AudioConfigEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void AudioConfigEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    buttonRefresh->setBounds ((getWidth() / 2) - (152 / 2), 312, 152, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void AudioConfigEditor::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == buttonRefresh.get())
    {
        //[UserButtonCode_buttonRefresh] -- add your button handler code here..
        //[/UserButtonCode_buttonRefresh]
    }

    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="AudioConfigEditor" componentName=""
                 parentClasses="public juce::AudioProcessorEditor" constructorParams="AudioConfigAudioProcessor&amp; p"
                 variableInitialisers="AudioProcessorEditor (&amp;p), audioProcessor (p), configManager(AudioConfig&quot;) "
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="600" initialHeight="400">
  <BACKGROUND backgroundColour="ff323e44"/>
  <LABEL name="" id="19e4f10940a85190" memberName="labelOutputDevice"
         virtualName="" explicitFocusOrder="0" pos="16 40 568 24" bkgCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="(placeholder)" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="6464e0074aa9cb40" memberName="labelOutputDeviceCaption"
         virtualName="" explicitFocusOrder="0" pos="16 16 568 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Output Device" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTBUTTON name="" id="3e5cfd8f1bfca5c0" memberName="buttonRefresh" virtualName=""
              explicitFocusOrder="0" pos="0Cc 312 152 24" buttonText="Refresh"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <LABEL name="" id="5cc3f4cbf6d921fd" memberName="labelInputDevice" virtualName=""
         explicitFocusOrder="0" pos="16 104 568 24" bkgCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="(placeholder)" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="27277f7c8bf37fbe" memberName="labelInputDeviceCaption"
         virtualName="" explicitFocusOrder="0" pos="16 80 568 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Input Device" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="ce602c03c12850f8" memberName="labelBufferSize" virtualName=""
         explicitFocusOrder="0" pos="312 168 272 16" bkgCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="(placeholder)" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="e89d95b8f47782e1" memberName="labelBufferSizeCaption"
         virtualName="" explicitFocusOrder="0" pos="312 144 272 16" edTextCol="ff000000"
         edBkgCol="0" labelText="Buffer Size&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="e9733b5a8c1838fa" memberName="labelSampleRate" virtualName=""
         explicitFocusOrder="0" pos="16 168 272 16" bkgCol="ff000000"
         edTextCol="ff000000" edBkgCol="0" labelText="(placeholder)" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="191a327503f70e3d" memberName="labelSampleRateCaption"
         virtualName="" explicitFocusOrder="0" pos="16 144 272 16" edTextCol="ff000000"
         edBkgCol="0" labelText="Buffer Size&#10;" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

