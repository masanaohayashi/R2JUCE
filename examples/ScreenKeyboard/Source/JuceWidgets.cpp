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

#include "JuceWidgets.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
JuceWidgets::JuceWidgets ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    label.reset (new juce::Label (juce::String(),
                                  TRANS ("JUCE (don\'t support that...)")));
    addAndMakeVisible (label.get());
    label->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    label->setJustificationType (juce::Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    textEditor.reset (new juce::TextEditor (juce::String()));
    addAndMakeVisible (textEditor.get());
    textEditor->setMultiLine (false);
    textEditor->setReturnKeyStartsNewLine (false);
    textEditor->setReadOnly (false);
    textEditor->setScrollbarsShown (true);
    textEditor->setCaretVisible (true);
    textEditor->setPopupMenuEnabled (true);
    textEditor->setText (juce::String());

    slider.reset (new juce::Slider (juce::String()));
    addAndMakeVisible (slider.get());
    slider->setRange (0, 10, 0);
    slider->setSliderStyle (juce::Slider::LinearHorizontal);
    slider->setTextBoxStyle (juce::Slider::TextBoxRight, false, 80, 20);
    slider->addListener (this);

    knob.reset (new juce::Slider (juce::String()));
    addAndMakeVisible (knob.get());
    knob->setRange (0, 10, 0);
    knob->setSliderStyle (juce::Slider::Rotary);
    knob->setTextBoxStyle (juce::Slider::TextBoxAbove, false, 80, 20);
    knob->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (264, 264);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

JuceWidgets::~JuceWidgets()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    textEditor = nullptr;
    slider = nullptr;
    knob = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void JuceWidgets::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void JuceWidgets::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    label->setBounds ((getWidth() / 2) - ((getWidth() - 0) / 2), 0, getWidth() - 0, 24);
    textEditor->setBounds ((getWidth() / 2) - ((getWidth() - 0) / 2), 32, getWidth() - 0, 24);
    slider->setBounds (getWidth() - 264, 64, 264, 24);
    knob->setBounds ((getWidth() / 2) - (88 / 2), 96, 88, 120);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void JuceWidgets::sliderValueChanged (juce::Slider* sliderThatWasMoved)
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

<JUCER_COMPONENT documentType="Component" className="JuceWidgets" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="264" initialHeight="264">
  <BACKGROUND backgroundColour="ff323e44"/>
  <LABEL name="" id="1ded676a2c1dea14" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0Cc 0 0M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="JUCE (don't support that...)" editableSingleClick="0"
         editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
         fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="33"/>
  <TEXTEDITOR name="" id="bbbdd1760ae1ac81" memberName="textEditor" virtualName=""
              explicitFocusOrder="0" pos="0Cc 32 0M 24" initialText="" multiline="0"
              retKeyStartsLine="0" readonly="0" scrollbars="1" caret="1" popupmenu="1"/>
  <SLIDER name="" id="4991501727486111" memberName="slider" virtualName=""
          explicitFocusOrder="0" pos="0Rr 64 264 24" min="0.0" max="10.0"
          int="0.0" style="LinearHorizontal" textBoxPos="TextBoxRight"
          textBoxEditable="1" textBoxWidth="80" textBoxHeight="20" skewFactor="1.0"
          needsCallback="1"/>
  <SLIDER name="" id="e7037bbf5efef38" memberName="knob" virtualName=""
          explicitFocusOrder="0" pos="0Cc 96 88 120" min="0.0" max="10.0"
          int="0.0" style="Rotary" textBoxPos="TextBoxAbove" textBoxEditable="1"
          textBoxWidth="80" textBoxHeight="20" skewFactor="1.0" needsCallback="1"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

