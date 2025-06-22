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

#include "R2AlertComponent.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
namespace r2juce {
//[/MiscUserDefs]

//==============================================================================
R2AlertComponent::R2AlertComponent (juce::Component* parent, const juce::String& title, const juce::String& message, const juce::StringArray& buttonLabels, std::function<void(int)> callback)
: parentComponent (parent), onResult (callback)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]
    
    labelTitle.reset (new juce::Label (juce::String(),
                                       TRANS ("(Placeholder)")));
    addAndMakeVisible (labelTitle.get());
    labelTitle->setFont (juce::Font (juce::FontOptions (20.00f, juce::Font::plain)));
    labelTitle->setJustificationType (juce::Justification::centredLeft);
    labelTitle->setEditable (false, false, false);
    labelTitle->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelTitle->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    
    labelMessage.reset (new juce::Label (juce::String(),
                                         TRANS ("label text\n"
                                                "asdasd\n")));
    addAndMakeVisible (labelMessage.get());
    labelMessage->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelMessage->setJustificationType (juce::Justification::topLeft);
    labelMessage->setEditable (false, false, false);
    labelMessage->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelMessage->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));
    
    button1.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (button1.get());
    button1->setButtonText (TRANS ("1"));
    button1->addListener (this);
    button1->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff505050));
    
    button2.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (button2.get());
    button2->setButtonText (TRANS ("2"));
    button2->addListener (this);
    button2->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff505050));
    
    button3.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (button3.get());
    button3->setButtonText (TRANS ("3"));
    button3->addListener (this);
    button3->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff505050));
    
    
    //[UserPreSize]
    jassert (parent != nullptr);
    parent->addAndMakeVisible (this);
    
    labelTitle->setText (title, juce::dontSendNotification);
    labelMessage->setText (message, juce::dontSendNotification);
    numButtons = buttonLabels.size();
    for (int i = 0; i < std::min(3, numButtons); i++)
    {
        juce::TextButton* button = nullptr;
        if (i == 0) button = button1.get();
        else if (i == 1) button = button2.get();
        else if (i == 2) button = button3.get();
        
        button->setButtonText (buttonLabels[i]);
    }
    
    //[/UserPreSize]
    
    setSize (568, 320);
    
    
    //[Constructor] You can add your own custom stuff here..
    setSize (parent->getWidth(), parent->getHeight());
    setTopLeftPosition(0, 0);
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();
    updateButtonFocus();
    //[/Constructor]
}

R2AlertComponent::~R2AlertComponent()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]
    
    labelTitle = nullptr;
    labelMessage = nullptr;
    button1 = nullptr;
    button2 = nullptr;
    button3 = nullptr;
    
    
    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void R2AlertComponent::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]
    
    g.fillAll (juce::Colour (0xc0000000));
    
    {
        int x = (getWidth() / 2) - (520 / 2), y = (getHeight() / 2) - (168 / 2), width = 520, height = 168;
        juce::Colour fillColour = juce::Colour (0xff323e44);
        //[UserPaintCustomArguments] Customize the painting arguments here..
        //[/UserPaintCustomArguments]
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
    }
    
    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void R2AlertComponent::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]
    
    labelTitle->setBounds ((getWidth() / 2) - (488 / 2), (getHeight() / 2) + -80, 488, 32);
    labelMessage->setBounds ((getWidth() / 2) - (488 / 2), (getHeight() / 2) - (80 / 2), 488, 80);
    button1->setBounds ((getWidth() / 2) + -144 - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
    button2->setBounds ((getWidth() / 2) - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
    button3->setBounds ((getWidth() / 2) + 144 - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
    //[UserResized] Add your own custom resize handling here..
    if (numButtons == 1)
    {
        button1->setBounds ((getWidth() / 2) - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
        button2->setVisible (false);
        button3->setVisible (false);
    }
    else if (numButtons == 2)
    {
        button1->setBounds ((getWidth() / 2) + -80 - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
        button2->setBounds ((getWidth() / 2) + 80 - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
        button3->setVisible (false);
    }
    //[/UserResized]
}

void R2AlertComponent::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]
    
    if (buttonThatWasClicked == button1.get())
    {
        //[UserButtonCode_button1] -- add your button handler code here..
        buttonClicked (1);
        //[/UserButtonCode_button1]
    }
    else if (buttonThatWasClicked == button2.get())
    {
        //[UserButtonCode_button2] -- add your button handler code here..
        buttonClicked (2);
        //[/UserButtonCode_button2]
    }
    else if (buttonThatWasClicked == button3.get())
    {
        //[UserButtonCode_button3] -- add your button handler code here..
        buttonClicked (3);
        //[/UserButtonCode_button3]
    }
    
    //[UserbuttonClicked_Post]
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
bool R2AlertComponent::keyPressed (const juce::KeyPress& key)
{
    if ((key == juce::KeyPress::spaceKey) || (key == juce::KeyPress::returnKey)) // Space/Enter
    {
        buttonClicked (selectedButtonIndex);
        return true;
    }
    if (key == juce::KeyPress::escapeKey) // Esc
    {
        buttonClicked (numButtons); // Cancel, No, etc.
        return true;
    }
    if (key == juce::KeyPress::leftKey) // ←キー
    {
        selectedButtonIndex -= 1;
        if (selectedButtonIndex < 1) selectedButtonIndex = numButtons;
        updateButtonFocus();
        return true;
    }
    if (key == juce::KeyPress::rightKey) // →キー
    {
        selectedButtonIndex += 1;
        if (selectedButtonIndex > numButtons) selectedButtonIndex = 1;
        updateButtonFocus();
        return true;
    }
    
    return true;
}

void R2AlertComponent::buttonClicked (int index)
{
    if (onResult) onResult (index);
    getParentComponent()->removeChildComponent(this);
    delete this;
}

void R2AlertComponent::updateButtonFocus()
{
    auto dullBlue = juce::Colour(60, 105, 170); // くすんだ青
    auto normalGrey = juce::Colours::grey;
    
    button1->setColour (juce::TextButton::buttonColourId, selectedButtonIndex == 1 ? dullBlue : normalGrey);
    button2->setColour (juce::TextButton::buttonColourId, selectedButtonIndex == 2 ? dullBlue : normalGrey);
    button3->setColour (juce::TextButton::buttonColourId, selectedButtonIndex == 3 ? dullBlue : normalGrey);
    repaint();
}

void R2AlertComponent::forOK (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback)
{
    new R2AlertComponent (parent, title, message, { TRANS("OK") }, callback);
}

void R2AlertComponent::forYesNo (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback)
{
    new R2AlertComponent (parent, title, message, { TRANS("Yes"), TRANS("No") }, callback);
}

void R2AlertComponent::forYesNoCancel (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback)
{
    new R2AlertComponent (parent, title, message, { TRANS("Yes"), TRANS("No"), TRANS("Cancel") }, callback);
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --
 
 This is where the Projucer stores the metadata that describe this GUI layout, so
 make changes in here at your peril!
 
 BEGIN_JUCER_METADATA
 
 <JUCER_COMPONENT documentType="Component" className="R2AlertComponent" componentName=""
 parentClasses="public juce::Component" constructorParams="juce::Component* parent, const juce::String&amp; title, const juce::String&amp; message, const juce::StringArray&amp; buttonLabels, std::function&lt;void(int)&gt; callback"
 variableInitialisers="parentComponent (parent), onResult (callback)"
 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
 fixedSize="1" initialWidth="568" initialHeight="320">
 <BACKGROUND backgroundColour="c0000000">
 <RECT pos="0Cc 0Cc 520 168" fill="solid: ff323e44" hasStroke="0"/>
 </BACKGROUND>
 <LABEL name="" id="77b9c3f4dabcf403" memberName="labelTitle" virtualName=""
 explicitFocusOrder="0" pos="0Cc -80C 488 32" edTextCol="ff000000"
 edBkgCol="0" labelText="(Placeholder)" editableSingleClick="0"
 editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
 fontsize="20.0" kerning="0.0" bold="0" italic="0" justification="33"/>
 <LABEL name="" id="7e6f7a3568e8ae72" memberName="labelMessage" virtualName=""
 explicitFocusOrder="0" pos="0Cc 0Cc 488 80" edTextCol="ff000000"
 edBkgCol="0" labelText="label text&#10;asdasd&#10;" editableSingleClick="0"
 editableDoubleClick="0" focusDiscardsChanges="0" fontname="Default font"
 fontsize="15.0" kerning="0.0" bold="0" italic="0" justification="9"/>
 <TEXTBUTTON name="" id="2940563e6100fb09" memberName="button1" virtualName=""
 explicitFocusOrder="0" pos="-144Cc 60Cc 120 24" bgColOff="ff505050"
 buttonText="1" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
 <TEXTBUTTON name="" id="76c11804a4ff9a75" memberName="button2" virtualName=""
 explicitFocusOrder="0" pos="0Cc 60Cc 120 24" bgColOff="ff505050"
 buttonText="2" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
 <TEXTBUTTON name="" id="213a4c7b38492acb" memberName="button3" virtualName=""
 explicitFocusOrder="0" pos="144Cc 60Cc 120 24" bgColOff="ff505050"
 buttonText="3" connectedEdges="0" needsCallback="1" radioGroupId="0"/>
 </JUCER_COMPONENT>
 
 END_JUCER_METADATA
 */
#endif


//[EndFile] You can add extra defines here...
}   //  namespace r2juce
//[/EndFile]

