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

#include "ScreenKeyboardAudioProcessorEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
ScreenKeyboardAudioProcessorEditor::ScreenKeyboardAudioProcessorEditor (ScreenKeyboardAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    textEditor.reset (new r2juce::R2TextEditor (juce::String()));
    addAndMakeVisible (textEditor.get());
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (juce::String());

    label.reset (new juce::Label (juce::String(),
                                  TRANS ("Input something using screen keyboard")));
    addAndMakeVisible (label.get());
    label->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    label->setJustificationType (juce::Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));


    //[UserPreSize]
    setWantsKeyboardFocus(true);
    textEditor->getKeyboardParent = [this](){
        return this;
    };

    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ScreenKeyboardAudioProcessorEditor::~ScreenKeyboardAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    textEditor = nullptr;
    label = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void ScreenKeyboardAudioProcessorEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void ScreenKeyboardAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    textEditor->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 40, getWidth() - 32, 24);
    label->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 16, getWidth() - 32, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void ScreenKeyboardAudioProcessorEditor::mouseDown (const juce::MouseEvent &event)
{
    grabKeyboardFocus();
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="ScreenKeyboardAudioProcessorEditor"
                 componentName="" parentClasses="public juce::AudioProcessorEditor"
                 constructorParams="ScreenKeyboardAudioProcessor&amp; p" variableInitialisers="AudioProcessorEditor (&amp;p), audioProcessor (p)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="568" initialHeight="320">
  <BACKGROUND backgroundColour="ff323e44"/>
  <TEXTEDITOR name="" id="d3ab75d3ee28df2e" memberName="textEditor" virtualName="r2juce::R2TextEditor"
              explicitFocusOrder="0" pos="0Cc 40 32M 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="" id="48e4c4280397cfb9" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0Cc 16 32M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Input something using screen keyboard"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

