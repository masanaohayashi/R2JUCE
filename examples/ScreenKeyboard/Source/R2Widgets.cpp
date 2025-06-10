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

#include "R2Widgets.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
R2Widgets::R2Widgets ()
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
                                  TRANS ("R2JUCE (supports screen keyboard)")));
    addAndMakeVisible (label.get());
    label->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    label->setJustificationType (juce::Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    slider.reset (new r2juce::R2Slider (juce::String()));
    addAndMakeVisible (slider.get());
    slider->setRange (0, 10, 0);
    slider->setSliderStyle (juce::Slider::LinearHorizontal);
    slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    slider->addListener (this);

    knob.reset (new r2juce::R2Slider (juce::String()));
    addAndMakeVisible (knob.get());
    knob->setRange (0, 10, 0);
    knob->setSliderStyle (juce::Slider::Rotary);
    knob->setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 20);
    knob->addListener (this);

    labelKnob.reset (new juce::Label (juce::String(),
                                      TRANS ("This guy shows wrong keyboard")));
    addAndMakeVisible (labelKnob.get());
    labelKnob->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelKnob->setJustificationType (juce::Justification::centred);
    labelKnob->setEditable (false, false, false);
    labelKnob->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelKnob->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelKnob->setBounds (8, 104, 80, 88);

    labelKnob2.reset (new juce::Label (juce::String(),
                                       TRANS ("Can be fixed by specifying right parent")));
    addAndMakeVisible (labelKnob2.get());
    labelKnob2->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelKnob2->setJustificationType (juce::Justification::centred);
    labelKnob2->setEditable (false, false, false);
    labelKnob2->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelKnob2->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelKnob2->setBounds (176, 104, 80, 88);


    //[UserPreSize]
    setWantsKeyboardFocus(true);
    textEditor->setKeyboardParentCallback([this]() { return getParentComponent(); });
    slider->setKeyboardParentCallback([this]() { return getParentComponent(); });
    //[/UserPreSize]

    setSize (264, 264);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

R2Widgets::~R2Widgets()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    textEditor = nullptr;
    label = nullptr;
    slider = nullptr;
    knob = nullptr;
    labelKnob = nullptr;
    labelKnob2 = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void R2Widgets::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void R2Widgets::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    textEditor->setBounds (0, 32, getWidth() - 0, 24);
    label->setBounds ((getWidth() / 2) - ((getWidth() - 0) / 2), 0, getWidth() - 0, 24);
    slider->setBounds ((getWidth() / 2) - ((getWidth() - 0) / 2), 64, getWidth() - 0, 24);
    knob->setBounds ((getWidth() / 2) - (88 / 2), 96, 88, 120);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void R2Widgets::sliderValueChanged (juce::Slider* sliderThatWasMoved)
{
    //[UsersliderValueChanged_Pre]
    //[/UsersliderValueChanged_Pre]

    if (sliderThatWasMoved == slider.get())
    {
        //[UserSliderCode_slider] -- add your slider handling code here..
        //[/UserSliderCode_slider]
    }
    else if (sliderThatWasMoved == knob.get())
    {
        //[UserSliderCode_knob] -- add your slider handling code here..
        //[/UserSliderCode_knob]
    }

    //[UsersliderValueChanged_Post]
    //[/UsersliderValueChanged_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="R2Widgets" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="264" initialHeight="264">
  <BACKGROUND backgroundColour="ff323e44"/>
  <TEXTEDITOR name="" id="d3ab75d3ee28df2e" memberName="textEditor" virtualName="r2juce::R2TextEditor"
              explicitFocusOrder="0" pos="0 32 0M 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <LABEL name="" id="4bb406791d8d250f" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0Cc 0 0M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="R2JUCE (supports screen keyboard)" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <SLIDER name="" id="ed207941c82e33d5" memberName="slider" virtualName="r2juce::R2Slider"
          explicitFocusOrder="0" pos="0Cc 64 0M 24" min="0.0" max="10.0"
          int="0.0" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="" id="3a10c761a8943f07" memberName="knob" virtualName="r2juce::R2Slider"
          explicitFocusOrder="0" pos="0Cc 96 88 120" min="0.0" max="10.0"
          int="0.0" style="Rotary" textBoxPos="TextBoxAbove" textBoxEditable="1"
          textBoxWidth="80" textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
  <LABEL name="" id="230dca745abcc9f5" memberName="labelKnob" virtualName=""
         explicitFocusOrder="0" pos="8 104 80 88" edTextCol="ff000000"
         edBkgCol="0" labelText="This guy shows wrong keyboard" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="36"/>
  <LABEL name="" id="1783fd842fea48e3" memberName="labelKnob2" virtualName=""
         explicitFocusOrder="0" pos="176 104 80 88" edTextCol="ff000000"
         edBkgCol="0" labelText="Can be fixed by specifying right parent"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="36"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

