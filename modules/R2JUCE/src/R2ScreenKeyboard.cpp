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

#include "R2ScreenKeyboard.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
R2ScreenKeyboard::R2ScreenKeyboard ()
{
    //[Constructor_pre] You can add your own custom stuff here..
    textEditor = nullptr;
    shiftEnabled = false;
    numericEnabled = false;
    //[/Constructor_pre]

    buttonQ.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonQ.get());
    buttonQ->setButtonText (TRANS ("q"));
    buttonQ->addListener (this);

    buttonW.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonW.get());
    buttonW->setButtonText (TRANS ("w"));
    buttonW->addListener (this);

    buttonE.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonE.get());
    buttonE->setButtonText (TRANS ("e"));
    buttonE->addListener (this);

    buttonR.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonR.get());
    buttonR->setButtonText (TRANS ("r"));
    buttonR->addListener (this);

    buttonT.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonT.get());
    buttonT->setButtonText (TRANS ("t"));
    buttonT->addListener (this);

    buttonY.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonY.get());
    buttonY->setButtonText (TRANS ("y"));
    buttonY->addListener (this);

    buttonU.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonU.get());
    buttonU->setButtonText (TRANS ("u"));
    buttonU->addListener (this);

    buttonI.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonI.get());
    buttonI->setButtonText (TRANS ("i"));
    buttonI->addListener (this);

    buttonO.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonO.get());
    buttonO->setButtonText (TRANS ("o"));
    buttonO->addListener (this);

    buttonP.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonP.get());
    buttonP->setButtonText (TRANS ("p"));
    buttonP->addListener (this);

    buttonA.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonA.get());
    buttonA->setButtonText (TRANS ("a"));
    buttonA->addListener (this);

    buttonS.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonS.get());
    buttonS->setButtonText (TRANS ("s"));
    buttonS->addListener (this);

    buttonD.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonD.get());
    buttonD->setButtonText (TRANS ("d"));
    buttonD->addListener (this);

    buttonF.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonF.get());
    buttonF->setButtonText (TRANS ("f"));
    buttonF->addListener (this);

    buttonG.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonG.get());
    buttonG->setButtonText (TRANS ("g"));
    buttonG->addListener (this);

    buttonH.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonH.get());
    buttonH->setButtonText (TRANS ("h"));
    buttonH->addListener (this);

    buttonJ.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonJ.get());
    buttonJ->setButtonText (TRANS ("j"));
    buttonJ->addListener (this);

    buttonK.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonK.get());
    buttonK->setButtonText (TRANS ("k"));
    buttonK->addListener (this);

    buttonL.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonL.get());
    buttonL->setButtonText (TRANS ("l"));
    buttonL->addListener (this);

    buttonShift.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonShift.get());
    buttonShift->setButtonText (TRANS ("Shift"));
    buttonShift->addListener (this);

    buttonZ.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonZ.get());
    buttonZ->setButtonText (TRANS ("z"));
    buttonZ->addListener (this);

    buttonX.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonX.get());
    buttonX->setButtonText (TRANS ("x"));
    buttonX->addListener (this);

    buttonC.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonC.get());
    buttonC->setButtonText (TRANS ("c"));
    buttonC->addListener (this);

    buttonV.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonV.get());
    buttonV->setButtonText (TRANS ("v"));
    buttonV->addListener (this);

    buttonB.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonB.get());
    buttonB->setButtonText (TRANS ("b"));
    buttonB->addListener (this);

    buttonN.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonN.get());
    buttonN->setButtonText (TRANS ("n"));
    buttonN->addListener (this);

    buttonM.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonM.get());
    buttonM->setButtonText (TRANS ("m"));
    buttonM->addListener (this);

    buttonBackspace.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonBackspace.get());
    buttonBackspace->setButtonText (TRANS ("<X]"));
    buttonBackspace->addListener (this);

    buttonNumeric.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonNumeric.get());
    buttonNumeric->setButtonText (TRANS ("123"));
    buttonNumeric->addListener (this);

    buttonSpace.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonSpace.get());
    buttonSpace->addListener (this);

    buttonEnter.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonEnter.get());
    buttonEnter->setButtonText (TRANS ("Enter"));
    buttonEnter->addListener (this);

    buttonCursorLeft.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonCursorLeft.get());
    buttonCursorLeft->setButtonText (TRANS ("<-"));
    buttonCursorLeft->addListener (this);

    buttonCursorRight.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (buttonCursorRight.get());
    buttonCursorRight->setButtonText (TRANS ("->"));
    buttonCursorRight->addListener (this);


    //[UserPreSize]
    //[/UserPreSize]

    setSize (568, 152);


    //[Constructor] You can add your own custom stuff here..
    setWantsKeyboardFocus (false);
    setMouseClickGrabsKeyboardFocus (false);

    for (int i = 0; i < getNumChildComponents(); i++)
    {
        auto comp = dynamic_cast<juce::TextButton*>(getChildComponent(i));
        if (comp != nullptr)
        {
            comp->setWantsKeyboardFocus (false);
            comp->setMouseClickGrabsKeyboardFocus (false);
            comp->setTriggeredOnMouseDown (true);
            comp->setRepeatSpeed (500, 100);
        }
    }
    buttonSpace->setButtonText (juce::String(" "));
    //[/Constructor]
}

R2ScreenKeyboard::~R2ScreenKeyboard()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    //[/Destructor_pre]

    buttonQ = nullptr;
    buttonW = nullptr;
    buttonE = nullptr;
    buttonR = nullptr;
    buttonT = nullptr;
    buttonY = nullptr;
    buttonU = nullptr;
    buttonI = nullptr;
    buttonO = nullptr;
    buttonP = nullptr;
    buttonA = nullptr;
    buttonS = nullptr;
    buttonD = nullptr;
    buttonF = nullptr;
    buttonG = nullptr;
    buttonH = nullptr;
    buttonJ = nullptr;
    buttonK = nullptr;
    buttonL = nullptr;
    buttonShift = nullptr;
    buttonZ = nullptr;
    buttonX = nullptr;
    buttonC = nullptr;
    buttonV = nullptr;
    buttonB = nullptr;
    buttonN = nullptr;
    buttonM = nullptr;
    buttonBackspace = nullptr;
    buttonNumeric = nullptr;
    buttonSpace = nullptr;
    buttonEnter = nullptr;
    buttonCursorLeft = nullptr;
    buttonCursorRight = nullptr;


    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void R2ScreenKeyboard::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colour (0xff424242));

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void R2ScreenKeyboard::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    buttonQ->setBounds ((getWidth() / 2) + -252 - (48 / 2), 16, 48, 24);
    buttonW->setBounds ((getWidth() / 2) + -196 - (48 / 2), 16, 48, 24);
    buttonE->setBounds ((getWidth() / 2) + -140 - (48 / 2), 16, 48, 24);
    buttonR->setBounds ((getWidth() / 2) + -84 - (48 / 2), 16, 48, 24);
    buttonT->setBounds ((getWidth() / 2) + -28 - (48 / 2), 16, 48, 24);
    buttonY->setBounds ((getWidth() / 2) + 28 - (48 / 2), 16, 48, 24);
    buttonU->setBounds ((getWidth() / 2) + 84 - (48 / 2), 16, 48, 24);
    buttonI->setBounds ((getWidth() / 2) + 116, 16, 48, 24);
    buttonO->setBounds ((getWidth() / 2) + 196 - (48 / 2), 16, 48, 24);
    buttonP->setBounds ((getWidth() / 2) + 252 - (48 / 2), 16, 48, 24);
    buttonA->setBounds ((getWidth() / 2) + -220 - (48 / 2), 48, 48, 24);
    buttonS->setBounds ((getWidth() / 2) + -164 - (48 / 2), 48, 48, 24);
    buttonD->setBounds ((getWidth() / 2) + -108 - (48 / 2), 48, 48, 24);
    buttonF->setBounds ((getWidth() / 2) + -52 - (48 / 2), 48, 48, 24);
    buttonG->setBounds ((getWidth() / 2) + 4 - (48 / 2), 48, 48, 24);
    buttonH->setBounds ((getWidth() / 2) + 60 - (48 / 2), 48, 48, 24);
    buttonJ->setBounds ((getWidth() / 2) + 116 - (48 / 2), 48, 48, 24);
    buttonK->setBounds ((getWidth() / 2) + 172 - (48 / 2), 48, 48, 24);
    buttonL->setBounds ((getWidth() / 2) + 228 - (48 / 2), 48, 48, 24);
    buttonShift->setBounds ((getWidth() / 2) + -240 - (72 / 2), 80, 72, 24);
    buttonZ->setBounds ((getWidth() / 2) + -164 - (48 / 2), 80, 48, 24);
    buttonX->setBounds ((getWidth() / 2) + -108 - (48 / 2), 80, 48, 24);
    buttonC->setBounds ((getWidth() / 2) + -52 - (48 / 2), 80, 48, 24);
    buttonV->setBounds ((getWidth() / 2) + 4 - (48 / 2), 80, 48, 24);
    buttonB->setBounds ((getWidth() / 2) + 60 - (48 / 2), 80, 48, 24);
    buttonN->setBounds ((getWidth() / 2) + 116 - (48 / 2), 80, 48, 24);
    buttonM->setBounds ((getWidth() / 2) + 172 - (48 / 2), 80, 48, 24);
    buttonBackspace->setBounds ((getWidth() / 2) + 140 - (48 / 2), 112, 48, 24);
    buttonNumeric->setBounds ((getWidth() / 2) + -240 - (72 / 2), 112, 72, 24);
    buttonSpace->setBounds ((getWidth() / 2) + -44 - (304 / 2), 112, 304, 24);
    buttonEnter->setBounds ((getWidth() / 2) + 240 - (72 / 2), 80, 72, 24);
    buttonCursorLeft->setBounds ((getWidth() / 2) + 196 - (48 / 2), 112, 48, 24);
    buttonCursorRight->setBounds ((getWidth() / 2) + 252 - (48 / 2), 112, 48, 24);
    //[UserResized] Add your own custom resize handling here..
    //[/UserResized]
}

void R2ScreenKeyboard::buttonClicked (juce::Button* buttonThatWasClicked)
{
    //[UserbuttonClicked_Pre]
    //[/UserbuttonClicked_Pre]

    if (buttonThatWasClicked == buttonQ.get())
    {
        //[UserButtonCode_buttonQ] -- add your button handler code here..
        //[/UserButtonCode_buttonQ]
    }
    else if (buttonThatWasClicked == buttonW.get())
    {
        //[UserButtonCode_buttonW] -- add your button handler code here..
        //[/UserButtonCode_buttonW]
    }
    else if (buttonThatWasClicked == buttonE.get())
    {
        //[UserButtonCode_buttonE] -- add your button handler code here..
        //[/UserButtonCode_buttonE]
    }
    else if (buttonThatWasClicked == buttonR.get())
    {
        //[UserButtonCode_buttonR] -- add your button handler code here..
        //[/UserButtonCode_buttonR]
    }
    else if (buttonThatWasClicked == buttonT.get())
    {
        //[UserButtonCode_buttonT] -- add your button handler code here..
        //[/UserButtonCode_buttonT]
    }
    else if (buttonThatWasClicked == buttonY.get())
    {
        //[UserButtonCode_buttonY] -- add your button handler code here..
        //[/UserButtonCode_buttonY]
    }
    else if (buttonThatWasClicked == buttonU.get())
    {
        //[UserButtonCode_buttonU] -- add your button handler code here..
        //[/UserButtonCode_buttonU]
    }
    else if (buttonThatWasClicked == buttonI.get())
    {
        //[UserButtonCode_buttonI] -- add your button handler code here..
        //[/UserButtonCode_buttonI]
    }
    else if (buttonThatWasClicked == buttonO.get())
    {
        //[UserButtonCode_buttonO] -- add your button handler code here..
        //[/UserButtonCode_buttonO]
    }
    else if (buttonThatWasClicked == buttonP.get())
    {
        //[UserButtonCode_buttonP] -- add your button handler code here..
        //[/UserButtonCode_buttonP]
    }
    else if (buttonThatWasClicked == buttonA.get())
    {
        //[UserButtonCode_buttonA] -- add your button handler code here..
        //[/UserButtonCode_buttonA]
    }
    else if (buttonThatWasClicked == buttonS.get())
    {
        //[UserButtonCode_buttonS] -- add your button handler code here..
        //[/UserButtonCode_buttonS]
    }
    else if (buttonThatWasClicked == buttonD.get())
    {
        //[UserButtonCode_buttonD] -- add your button handler code here..
        //[/UserButtonCode_buttonD]
    }
    else if (buttonThatWasClicked == buttonF.get())
    {
        //[UserButtonCode_buttonF] -- add your button handler code here..
        //[/UserButtonCode_buttonF]
    }
    else if (buttonThatWasClicked == buttonG.get())
    {
        //[UserButtonCode_buttonG] -- add your button handler code here..
        //[/UserButtonCode_buttonG]
    }
    else if (buttonThatWasClicked == buttonH.get())
    {
        //[UserButtonCode_buttonH] -- add your button handler code here..
        //[/UserButtonCode_buttonH]
    }
    else if (buttonThatWasClicked == buttonJ.get())
    {
        //[UserButtonCode_buttonJ] -- add your button handler code here..
        //[/UserButtonCode_buttonJ]
    }
    else if (buttonThatWasClicked == buttonK.get())
    {
        //[UserButtonCode_buttonK] -- add your button handler code here..
        //[/UserButtonCode_buttonK]
    }
    else if (buttonThatWasClicked == buttonL.get())
    {
        //[UserButtonCode_buttonL] -- add your button handler code here..
        //[/UserButtonCode_buttonL]
    }
    else if (buttonThatWasClicked == buttonShift.get())
    {
        //[UserButtonCode_buttonShift] -- add your button handler code here..
        shiftEnabled = !shiftEnabled;
        updateButtons();
        return;
        //[/UserButtonCode_buttonShift]
    }
    else if (buttonThatWasClicked == buttonZ.get())
    {
        //[UserButtonCode_buttonZ] -- add your button handler code here..
        //[/UserButtonCode_buttonZ]
    }
    else if (buttonThatWasClicked == buttonX.get())
    {
        //[UserButtonCode_buttonX] -- add your button handler code here..
        //[/UserButtonCode_buttonX]
    }
    else if (buttonThatWasClicked == buttonC.get())
    {
        //[UserButtonCode_buttonC] -- add your button handler code here..
        //[/UserButtonCode_buttonC]
    }
    else if (buttonThatWasClicked == buttonV.get())
    {
        //[UserButtonCode_buttonV] -- add your button handler code here..
        //[/UserButtonCode_buttonV]
    }
    else if (buttonThatWasClicked == buttonB.get())
    {
        //[UserButtonCode_buttonB] -- add your button handler code here..
        //[/UserButtonCode_buttonB]
    }
    else if (buttonThatWasClicked == buttonN.get())
    {
        //[UserButtonCode_buttonN] -- add your button handler code here..
        //[/UserButtonCode_buttonN]
    }
    else if (buttonThatWasClicked == buttonM.get())
    {
        //[UserButtonCode_buttonM] -- add your button handler code here..
        //[/UserButtonCode_buttonM]
    }
    else if (buttonThatWasClicked == buttonBackspace.get())
    {
        //[UserButtonCode_buttonBackspace] -- add your button handler code here..
        if (textEditor != nullptr)
        {
            textEditor->deleteBackwards(false);
        }
        return;
        //[/UserButtonCode_buttonBackspace]
    }
    else if (buttonThatWasClicked == buttonNumeric.get())
    {
        //[UserButtonCode_buttonNumeric] -- add your button handler code here..
        numericEnabled = !numericEnabled;
        shiftEnabled = false;
        updateButtons();
        return;
        //[/UserButtonCode_buttonNumeric]
    }
    else if (buttonThatWasClicked == buttonSpace.get())
    {
        //[UserButtonCode_buttonSpace] -- add your button handler code here..
        //[/UserButtonCode_buttonSpace]
    }
    else if (buttonThatWasClicked == buttonEnter.get())
    {
        //[UserButtonCode_buttonEnter] -- add your button handler code here..
        if (textEditor != nullptr)
        {
            textEditor->keyPressed(juce::KeyPress(juce::KeyPress::returnKey));
            textEditor->giveAwayKeyboardFocus();
            if (enterCallback != nullptr) {
                enterCallback();
            }
        }
        return;
        //[/UserButtonCode_buttonEnter]
    }
    else if (buttonThatWasClicked == buttonCursorLeft.get())
    {
        //[UserButtonCode_buttonCursorLeft] -- add your button handler code here..
        if (textEditor != nullptr)
        {
            textEditor->moveCaretLeft(false, false);
        }
        return;
        //[/UserButtonCode_buttonCursorLeft]
    }
    else if (buttonThatWasClicked == buttonCursorRight.get())
    {
        //[UserButtonCode_buttonCursorRight] -- add your button handler code here..
        if (textEditor != nullptr)
        {
            textEditor->moveCaretRight(false, false);
        }
        return;
        //[/UserButtonCode_buttonCursorRight]
    }

    //[UserbuttonClicked_Post]
    auto buttonKey = dynamic_cast<juce::TextButton*>(buttonThatWasClicked);
    if (buttonKey != nullptr)
    {
        if (textEditor != nullptr)
        {
            textEditor->insertTextAtCaret (buttonKey->getButtonText());
        }

        for (auto itr = listeners.cbegin(), end_ = listeners.cend(); itr != end_; itr++)
        {
            (*itr)->onSoftKeyboardKeyPressed (this, buttonKey->getButtonText());
        }
    }
    //[/UserbuttonClicked_Post]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
void R2ScreenKeyboard::setTextEditor(juce::TextEditor* editor)
{
    textEditor = editor;
}

void R2ScreenKeyboard::addListener(R2ScreenKeyboardListener* listener)
{
    listeners.push_back (listener);
}

void R2ScreenKeyboard::removeListener(R2ScreenKeyboardListener* listener)
{
    listeners.remove (listener);
}

void R2ScreenKeyboard::updateButtons()
{
    if (shiftEnabled)
    {
        if (numericEnabled)
        {
            buttonQ->setButtonText(juce::String("["));
            buttonW->setButtonText(juce::String("]"));
            buttonE->setButtonText(juce::String("{"));
            buttonR->setButtonText(juce::String("}"));
            buttonT->setButtonText(juce::String("#"));
            buttonY->setButtonText(juce::String("%"));
            buttonU->setButtonText(juce::String("^"));
            buttonI->setButtonText(juce::String("*"));
            buttonO->setButtonText(juce::String("+"));
            buttonP->setButtonText(juce::String("="));

            buttonA->setButtonText(juce::String("_"));
            buttonS->setButtonText(juce::String("\\"));
            buttonD->setButtonText(juce::String("|"));
            buttonF->setButtonText(juce::String("~"));
            buttonG->setButtonText(juce::String("<"));
            buttonH->setButtonText(juce::String(">"));
            buttonJ->setButtonText(juce::String("$"));
            buttonK->setButtonText(juce::String(" "));
            buttonL->setButtonText(juce::String(" "));

            buttonZ->setButtonText(juce::String("."));
            buttonX->setButtonText(juce::String(","));
            buttonC->setButtonText(juce::String("?"));
            buttonV->setButtonText(juce::String("!"));
            buttonB->setButtonText(juce::String("'"));
            buttonN->setButtonText(juce::String("\""));
            buttonM->setButtonText(juce::String("@"));

            buttonShift->setButtonText(juce::String("123"));
            buttonNumeric->setButtonText(juce::String("ABC"));
        }
        else
        {
            buttonQ->setButtonText(juce::String("Q"));
            buttonW->setButtonText(juce::String("W"));
            buttonE->setButtonText(juce::String("E"));
            buttonR->setButtonText(juce::String("R"));
            buttonT->setButtonText(juce::String("T"));
            buttonY->setButtonText(juce::String("Y"));
            buttonU->setButtonText(juce::String("U"));
            buttonI->setButtonText(juce::String("I"));
            buttonO->setButtonText(juce::String("O"));
            buttonP->setButtonText(juce::String("P"));

            buttonA->setButtonText(juce::String("A"));
            buttonS->setButtonText(juce::String("S"));
            buttonD->setButtonText(juce::String("D"));
            buttonF->setButtonText(juce::String("F"));
            buttonG->setButtonText(juce::String("G"));
            buttonH->setButtonText(juce::String("H"));
            buttonJ->setButtonText(juce::String("J"));
            buttonK->setButtonText(juce::String("K"));
            buttonL->setButtonText(juce::String("L"));

            buttonZ->setButtonText(juce::String("Z"));
            buttonX->setButtonText(juce::String("X"));
            buttonC->setButtonText(juce::String("C"));
            buttonV->setButtonText(juce::String("V"));
            buttonB->setButtonText(juce::String("B"));
            buttonN->setButtonText(juce::String("N"));
            buttonM->setButtonText(juce::String("M"));

            buttonShift->setButtonText(juce::String("Shift"));
            buttonNumeric->setButtonText(juce::String("123"));
        }
    }
    else
    {
        if (numericEnabled)
        {
            buttonQ->setButtonText(juce::String("1"));
            buttonW->setButtonText(juce::String("2"));
            buttonE->setButtonText(juce::String("3"));
            buttonR->setButtonText(juce::String("4"));
            buttonT->setButtonText(juce::String("5"));
            buttonY->setButtonText(juce::String("6"));
            buttonU->setButtonText(juce::String("7"));
            buttonI->setButtonText(juce::String("8"));
            buttonO->setButtonText(juce::String("9"));
            buttonP->setButtonText(juce::String("0"));

            buttonA->setButtonText(juce::String("-"));
            buttonS->setButtonText(juce::String("/"));
            buttonD->setButtonText(juce::String(":"));
            buttonF->setButtonText(juce::String(";"));
            buttonG->setButtonText(juce::String("("));
            buttonH->setButtonText(juce::String(")"));
            buttonJ->setButtonText(juce::String("&"));
            buttonK->setButtonText(juce::String("@"));
            buttonL->setButtonText(juce::String("\""));

            buttonZ->setButtonText(juce::String("."));
            buttonX->setButtonText(juce::String(","));
            buttonC->setButtonText(juce::String("?"));
            buttonV->setButtonText(juce::String("!"));
            buttonB->setButtonText(juce::String("'"));
            buttonN->setButtonText(juce::String(" "));
            buttonM->setButtonText(juce::String(" "));

            buttonShift->setButtonText(juce::String("#+="));
            buttonNumeric->setButtonText(juce::String("ABC"));
        }
        else
        {
            buttonQ->setButtonText(juce::String("q"));
            buttonW->setButtonText(juce::String("w"));
            buttonE->setButtonText(juce::String("e"));
            buttonR->setButtonText(juce::String("r"));
            buttonT->setButtonText(juce::String("t"));
            buttonY->setButtonText(juce::String("y"));
            buttonU->setButtonText(juce::String("u"));
            buttonI->setButtonText(juce::String("i"));
            buttonO->setButtonText(juce::String("o"));
            buttonP->setButtonText(juce::String("p"));

            buttonA->setButtonText(juce::String("a"));
            buttonS->setButtonText(juce::String("s"));
            buttonD->setButtonText(juce::String("d"));
            buttonF->setButtonText(juce::String("f"));
            buttonG->setButtonText(juce::String("g"));
            buttonH->setButtonText(juce::String("h"));
            buttonJ->setButtonText(juce::String("j"));
            buttonK->setButtonText(juce::String("k"));
            buttonL->setButtonText(juce::String("l"));

            buttonZ->setButtonText(juce::String("z"));
            buttonX->setButtonText(juce::String("x"));
            buttonC->setButtonText(juce::String("c"));
            buttonV->setButtonText(juce::String("v"));
            buttonB->setButtonText(juce::String("b"));
            buttonN->setButtonText(juce::String("n"));
            buttonM->setButtonText(juce::String("m"));

            buttonShift->setButtonText(juce::String("Shift"));
            buttonNumeric->setButtonText(juce::String("123"));
        }
    }
}
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="R2ScreenKeyboard" componentName=""
                 parentClasses="public juce::Component" constructorParams="" variableInitialisers=""
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="1" initialWidth="568" initialHeight="152">
  <BACKGROUND backgroundColour="ff424242"/>
  <TEXTBUTTON name="" id="21d5a2bc05878659" memberName="buttonQ" virtualName=""
              explicitFocusOrder="0" pos="-252Cc 16 48 24" buttonText="q" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="dc5101856e8defca" memberName="buttonW" virtualName=""
              explicitFocusOrder="0" pos="-196Cc 16 48 24" buttonText="w" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="23199b427b3bb07d" memberName="buttonE" virtualName=""
              explicitFocusOrder="0" pos="-140Cc 16 48 24" buttonText="e" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="15a652cd257f27a5" memberName="buttonR" virtualName=""
              explicitFocusOrder="0" pos="-84Cc 16 48 24" buttonText="r" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="c74c41adf7ccf948" memberName="buttonT" virtualName=""
              explicitFocusOrder="0" pos="-28Cc 16 48 24" buttonText="t" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="d28ad45ef3720d0e" memberName="buttonY" virtualName=""
              explicitFocusOrder="0" pos="28Cc 16 48 24" buttonText="y" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="601278ac1c7b4000" memberName="buttonU" virtualName=""
              explicitFocusOrder="0" pos="84Cc 16 48 24" buttonText="u" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="1b575d4468bfee93" memberName="buttonI" virtualName=""
              explicitFocusOrder="0" pos="116C 16 48 24" buttonText="i" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="9ed816871d7c9ae5" memberName="buttonO" virtualName=""
              explicitFocusOrder="0" pos="196Cc 16 48 24" buttonText="o" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="bc693ca199fb3346" memberName="buttonP" virtualName=""
              explicitFocusOrder="0" pos="252Cc 16 48 24" buttonText="p" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="cbf5ff756d76d8d5" memberName="buttonA" virtualName=""
              explicitFocusOrder="0" pos="-220Cc 48 48 24" buttonText="a" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="587ecc70ab1e3da7" memberName="buttonS" virtualName=""
              explicitFocusOrder="0" pos="-164Cc 48 48 24" buttonText="s" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="cca3c9319cbfce4e" memberName="buttonD" virtualName=""
              explicitFocusOrder="0" pos="-108Cc 48 48 24" buttonText="d" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="5c3c33797c48c48" memberName="buttonF" virtualName=""
              explicitFocusOrder="0" pos="-52Cc 48 48 24" buttonText="f" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="1cbc57716b26f21a" memberName="buttonG" virtualName=""
              explicitFocusOrder="0" pos="4Cc 48 48 24" buttonText="g" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="5c5773cd7aee1753" memberName="buttonH" virtualName=""
              explicitFocusOrder="0" pos="60Cc 48 48 24" buttonText="h" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="5316de0e526890ca" memberName="buttonJ" virtualName=""
              explicitFocusOrder="0" pos="116Cc 48 48 24" buttonText="j" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="4130d3e25afe9129" memberName="buttonK" virtualName=""
              explicitFocusOrder="0" pos="172Cc 48 48 24" buttonText="k" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="f52e1833ece70daf" memberName="buttonL" virtualName=""
              explicitFocusOrder="0" pos="228Cc 48 48 24" buttonText="l" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="6fcb01bd1183eb30" memberName="buttonShift" virtualName=""
              explicitFocusOrder="0" pos="-240Cc 80 72 24" buttonText="Shift"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="97c25ee038e57761" memberName="buttonZ" virtualName=""
              explicitFocusOrder="0" pos="-164Cc 80 48 24" buttonText="z" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="294242b78535b252" memberName="buttonX" virtualName=""
              explicitFocusOrder="0" pos="-108Cc 80 48 24" buttonText="x" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="80380f689bd02c3a" memberName="buttonC" virtualName=""
              explicitFocusOrder="0" pos="-52Cc 80 48 24" buttonText="c" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="11a251bd9cd37b7a" memberName="buttonV" virtualName=""
              explicitFocusOrder="0" pos="4Cc 80 48 24" buttonText="v" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="63ec41f3b3458edc" memberName="buttonB" virtualName=""
              explicitFocusOrder="0" pos="60Cc 80 48 24" buttonText="b" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="323ffdd2e1b4601b" memberName="buttonN" virtualName=""
              explicitFocusOrder="0" pos="116Cc 80 48 24" buttonText="n" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="8ff9cffd45d0caa7" memberName="buttonM" virtualName=""
              explicitFocusOrder="0" pos="172Cc 80 48 24" buttonText="m" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="5938188b11198aa8" memberName="buttonBackspace" virtualName=""
              explicitFocusOrder="0" pos="140Cc 112 48 24" buttonText="&lt;X]"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="d928aad53188a843" memberName="buttonNumeric" virtualName=""
              explicitFocusOrder="0" pos="-240Cc 112 72 24" buttonText="123"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="376db491eb95d31d" memberName="buttonSpace" virtualName=""
              explicitFocusOrder="0" pos="-44Cc 112 304 24" buttonText="" connectedEdges="0"
              needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="3101821f0b04721d" memberName="buttonEnter" virtualName=""
              explicitFocusOrder="0" pos="240Cc 80 72 24" buttonText="Enter"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="afa1b4226a1c6b75" memberName="buttonCursorLeft" virtualName=""
              explicitFocusOrder="0" pos="196Cc 112 48 24" buttonText="&lt;-"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
  <TEXTBUTTON name="" id="a23fba533b58e8e1" memberName="buttonCursorRight"
              virtualName="" explicitFocusOrder="0" pos="252Cc 112 48 24" buttonText="-&gt;"
              connectedEdges="0" needsCallback="1" radioGroupId="0"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

