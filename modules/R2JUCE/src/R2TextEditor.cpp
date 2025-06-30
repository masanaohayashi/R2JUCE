/*
  ==============================================================================

    R2TextEditor.cpp
    Created: 10 May 2022 8:36:15pm
    Author:  ring2

  ==============================================================================
*/

#include "R2TextEditor.h"

namespace r2juce {

static int test_variable_r2texteditor = 42;

R2TextEditor::R2TextEditor (const juce::String& componentName, wchar_t passwordCharacter)
: TextEditor (componentName, passwordCharacter)
{
    exitedWithEscape = false;
}

bool R2TextEditor::keyPressed(const juce::KeyPress& key)
{
    if (key == juce::KeyPress::escapeKey)
    {
        exitedWithEscape = true;
        giveAwayKeyboardFocus();
        return true;
    }
    
    if (key == juce::KeyPress::returnKey)
    {
        exitedWithEscape = false;
        giveAwayKeyboardFocus();
        return true;
    }
    
    return juce::TextEditor::keyPressed(key);
}

void R2TextEditor::focusGained (FocusChangeType type)
{
    juce::TextEditor::focusGained (type);

    if (focusChangedCallback != nullptr) {
        focusChangedCallback(true);
    }

    exitedWithEscape = false;

    if (!useScreenKeyboard) return;
    
    juce::Component* parent = nullptr;
    if (keyboardParentCallback != nullptr) {
        parent = keyboardParentCallback();
    }
    
    if (parent == nullptr) {
        parent = getParentComponent();
    }
    
    if (parent != nullptr) {
        keyboard.reset(new R2ScreenKeyboard);
        keyboard->setTextEditor(this);
        parent->addAndMakeVisible(keyboard.get());
        keyboard->setBounds(0,
                            parent->getHeight() - keyboard->getHeight(),
                            parent->getWidth(),
                            keyboard->getHeight());
    }
}

void R2TextEditor::focusLost (FocusChangeType type)
{
    TextEditor::focusLost (type);
    
    if (focusChangedCallback != nullptr) {
        focusChangedCallback(false);
    }

    if (!useScreenKeyboard) return;

    if (keyboard != nullptr) {
        auto parent = keyboard->getParentComponent();
        parent->removeChildComponent(keyboard.get());
        keyboard = nullptr;
    }
}

}   //  namespace r2juce
