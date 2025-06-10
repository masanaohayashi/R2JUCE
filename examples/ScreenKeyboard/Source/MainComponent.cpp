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

#include "MainComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
MainComponent::MainComponent ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]

    label.reset (new juce::Label (juce::String(),
                                  TRANS ("Screen keyboard is enabled for R2JUCE widgets on Raspberry Pi")));
    addAndMakeVisible (label.get());
    label->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    label->setJustificationType (juce::Justification::centredLeft);
    label->setEditable (false, false, false);
    label->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    label->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    r2Widgets.reset (new R2Widgets());
    addAndMakeVisible (r2Widgets.get());
    r2Widgets->setBounds (16, 40, 264, 264);

    juceWidgets.reset (new JuceWidgets());
    addAndMakeVisible (juceWidgets.get());

    //[UserPreSize]
    //[/UserPreSize]

    setSize (568, 320);


    //[Constructor] You can add your own custom stuff here..
    //[/Constructor]
}

MainComponent::~MainComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    label = nullptr;
    r2Widgets = nullptr;
    juceWidgets = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff323e44));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void MainComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    label->setBounds ((getWidth() / 2) - ((getWidth() - 32) / 2), 8, getWidth() - 32, 24);
    juceWidgets->setBounds (getWidth() - 16 - 264, 40, 264, 264);
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

<JUCER_COMPONENT documentType="Component" className="MainComponent" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="568" initialHeight="320">
  <BACKGROUND backgroundColour="ff323e44"/>
  <LABEL name="" id="48e4c4280397cfb9" memberName="label" virtualName=""
         explicitFocusOrder="0" pos="0Cc 8 32M 24" edTextCol="ff000000"
         edBkgCol="0" labelText="Screen keyboard is enabled for R2JUCE widgets on Raspberry Pi"
         editableSingleClick="0" editableDoubleClick="0" focusDiscardsChanges="0"
         fontname="Default font" fontsize="15.0" kerning="0.0" bold="0"
         italic="0" justification="33"/>
  <JUCERCOMP name="" id="d8957cd045c74b" memberName="r2Widgets" virtualName=""
             explicitFocusOrder="0" pos="16 40 264 264" sourceFile="R2Widgets.cpp"
             constructorParams=""/>
  <JUCERCOMP name="" id="a25e48cded657280" memberName="juceWidgets" virtualName=""
             explicitFocusOrder="0" pos="16Rr 40 264 264" sourceFile="JuceWidgets.cpp"
             constructorParams=""/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

