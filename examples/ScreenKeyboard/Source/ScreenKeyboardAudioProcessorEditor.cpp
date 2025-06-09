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

    textEditorR2JUCE.reset (new r2juce::R2TextEditor (juce::String()));
    addAndMakeVisible (textEditorR2JUCE.get());
    textEditorR2JUCE->setMultiLine (false);
    textEditorR2JUCE->setReturnKeyStartsNewLine (false);
    textEditorR2JUCE->setReadOnly (false);
    textEditorR2JUCE->setScrollbarsShown (true);
    textEditorR2JUCE->setCaretVisible (true);
    textEditorR2JUCE->setPopupMenuEnabled (true);
    textEditorR2JUCE->setText (juce::String());

    label.reset (new juce::Label (juce::String(),
                                  TRANS ("Screen keyboard is enabled for R2JUCE widgets")));
    addAndMakeVisible (label.get());
    label->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    label->setJustificationType (juce::Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelR2JUCE.reset (new juce::Label (juce::String(),
                                        TRANS ("R2JUCE")));
    addAndMakeVisible (labelR2JUCE.get());
    labelR2JUCE->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelR2JUCE->setJustificationType (juce::Justification::centredLeft);
    labelR2JUCE->setEditable (false, false, false);
    labelR2JUCE->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelR2JUCE->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelJUCE.reset (new juce::Label (juce::String(),
                                      TRANS ("JUCE")));
    addAndMakeVisible (labelJUCE.get());
    labelJUCE->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelJUCE->setJustificationType (juce::Justification::centredLeft);
    labelJUCE->setEditable (false, false, false);
    labelJUCE->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelJUCE->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    textEditorJUCE.reset (new juce::TextEditor (juce::String()));
    addAndMakeVisible (textEditorJUCE.get());
    textEditorJUCE->setMultiLine (false);
    textEditorJUCE->setReturnKeyStartsNewLine (false);
    textEditorJUCE->setReadOnly (false);
    textEditorJUCE->setScrollbarsShown (true);
    textEditorJUCE->setCaretVisible (true);
    textEditorJUCE->setPopupMenuEnabled (true);
    textEditorJUCE->setText (juce::String());

    sliderR2JUCE.reset (new r2juce::R2Slider (juce::String()));
    addAndMakeVisible (sliderR2JUCE.get());
    sliderR2JUCE->setRange (0, 10, 0);
    sliderR2JUCE->setSliderStyle (juce::Slider::LinearHorizontal);
    sliderR2JUCE->setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    sliderR2JUCE->addListener (this);

    sliderJUCE.reset (new juce::Slider (juce::String()));
    addAndMakeVisible (sliderJUCE.get());
    sliderJUCE->setRange (0, 10, 0);
    sliderJUCE->setSliderStyle (juce::Slider::LinearHorizontal);
    sliderJUCE->setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    sliderJUCE->addListener (this);

    sliderR2JUCE2.reset (new r2juce::R2Slider (juce::String()));
    addAndMakeVisible (sliderR2JUCE2.get());
    sliderR2JUCE2->setRange (0, 10, 0);
    sliderR2JUCE2->setSliderStyle (juce::Slider::Rotary);
    sliderR2JUCE2->setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 20);
    sliderR2JUCE2->addListener (this);

    sliderJUCE2.reset (new juce::Slider (juce::String()));
    addAndMakeVisible (sliderJUCE2.get());
    sliderJUCE2->setRange (0, 10, 0);
    sliderJUCE2->setSliderStyle (juce::Slider::Rotary);
    sliderJUCE2->setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 20);
    sliderJUCE2->addListener (this);


    //[UserPreSize]
    setWantsKeyboardFocus(true);
    /*
    textEditor->getKeyboardParent = [this](){
        return this;
    };
*/
    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

ScreenKeyboardAudioProcessorEditor::~ScreenKeyboardAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    textEditorR2JUCE = nullptr;
    label = nullptr;
    labelR2JUCE = nullptr;
    labelJUCE = nullptr;
    textEditorJUCE = nullptr;
    sliderR2JUCE = nullptr;
    sliderJUCE = nullptr;
    sliderR2JUCE2 = nullptr;
    sliderJUCE2 = nullptr;


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

    textEditorR2JUCE->setBounds (16, 56, proportionOfWidth (0.4648f), 24);
    label->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 0, getWidth() - 32, 24);
    labelR2JUCE->setBounds (16, 32, proportionOfWidth (0.4613f), 24);
    labelJUCE->setBounds (getWidth() - 16 - proportionOfWidth (0.4648f), 32, proportionOfWidth (0.4648f), 24);
    textEditorJUCE->setBounds (getWidth() - 16 - proportionOfWidth (0.4648f), 56, proportionOfWidth (0.4648f), 24);
    sliderR2JUCE->setBounds (16, 88, proportionOfWidth (0.4648f), 24);
    sliderJUCE->setBounds (getWidth() - 16 - proportionOfWidth (0.4648f), 88, proportionOfWidth (0.4648f), 24);
    sliderR2JUCE2->setBounds (proportionOfWidth (0.1972f), 120, proportionOfWidth (0.1268f), 88);
    sliderJUCE2->setBounds (384, 120, proportionOfWidth (0.1268f), 88);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void ScreenKeyboardAudioProcessorEditor::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == sliderR2JUCE.get())
    {
        //[UserSliderCode_sliderR2JUCE] -- add your slider handling code here..
        //[/UserSliderCode_sliderR2JUCE]
    }
    else if (sliderThatWasMoved == sliderJUCE.get())
    {
        //[UserSliderCode_sliderJUCE] -- add your slider handling code here..
        //[/UserSliderCode_sliderJUCE]
    }
    else if (sliderThatWasMoved == sliderR2JUCE2.get())
    {
        //[UserSliderCode_sliderR2JUCE2] -- add your slider handling code here..
        //[/UserSliderCode_sliderR2JUCE2]
    }
    else if (sliderThatWasMoved == sliderJUCE2.get())
    {
        //[UserSliderCode_sliderJUCE2] -- add your slider handling code here..
        //[/UserSliderCode_sliderJUCE2]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
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
  <TEXTEDITOR name="" id="d3ab75d3ee28df2e" memberName="textEditorR2JUCE" virtualName="r2juce::R2TextEditor"
              explicitFocusOrder="0" pos="16 56 46.479% 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <LABEL name="" id="48e4c4280397cfb9" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0Cc 0 32M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Screen keyboard is enabled for R2JUCE widgets"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <LABEL name="" id="4bb406791d8d250f" memberName="labelR2JUCE" virtualName=""
         explicitFocusOrder="0" pos="16 32 46.127% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="R2JUCE" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <LABEL name="" id="1ded676a2c1dea14" memberName="labelJUCE" virtualName=""
         explicitFocusOrder="0" pos="16Rr 32 46.479% 24" edTextCol="ff000000"
         edBkgCol="0" labelText="JUCE" editableSingleClick="0" editableDoubleClick="0"
         focusDiscardsChanges="0" fontname="Default font" fontsize="15.0"
         kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="bbbdd1760ae1ac81" memberName="textEditorJUCE" virtualName=""
              explicitFocusOrder="0" pos="16Rr 56 46.479% 24" initialText=""
              multiline="0" retKeyStartsLine="0" readonly="0" scrollbars="1"
              caret="1" popupmenu="1"/>
  <SLIDER name="" id="ed207941c82e33d5" memberName="sliderR2JUCE" virtualName="r2juce::R2Slider"
          explicitFocusOrder="0" pos="16 88 46.479% 24" min="0.0" max="10.0"
          int="0.0" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="" id="4991501727486111" memberName="sliderJUCE" virtualName=""
          explicitFocusOrder="0" pos="16Rr 88 46.479% 24" min="0.0" max="10.0"
          int="0.0" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="" id="3a10c761a8943f07" memberName="sliderR2JUCE2" virtualName="r2juce::R2Slider"
          explicitFocusOrder="0" pos="19.718% 120 12.676% 88" min="0.0"
          max="10.0" int="0.0" style="Rotary" textBoxPos="TextBoxAbove"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="" id="e7037bbf5efef38" memberName="sliderJUCE2" virtualName=""
          explicitFocusOrder="0" pos="384 120 12.676% 88" min="0.0" max="10.0"
          int="0.0" style="Rotary" textBoxPos="TextBoxAbove" textBoxEditable="1"
          textBoxWidth="80" textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

