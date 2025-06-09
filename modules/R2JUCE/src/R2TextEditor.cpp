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
}

void R2TextEditor::focusGained (FocusChangeType type)
{
    juce::TextEditor::focusGained (type);

    if (!useScreenKeyboard) return;
    
    juce::Component* parent = nullptr;
    if (getKeyboardParent != nullptr) {
        parent = getKeyboardParent();
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
    
    if (focusChangedCallback != nullptr) {
        focusChangedCallback(true);
    }
    
    for (auto& i : listeners) {
        i->onR2TextEditorFocusGained (this);
    }
}

void R2TextEditor::focusLost (FocusChangeType type)
{
    TextEditor::focusLost (type);
    
    if (!useScreenKeyboard) return;

    if (keyboard != nullptr) {
        auto parent = keyboard->getParentComponent();
        parent->removeChildComponent(keyboard.get());
        keyboard = nullptr;
    }
    
    if (focusChangedCallback != nullptr) {
        focusChangedCallback(false);
    }
    
    for (auto& i : listeners) {
        i->onR2TextEditorFocusLost(this);
    }
}

void R2TextEditor::addListener(R2TextEditorListener* listener)
{
    listeners.push_back (listener);
}

void R2TextEditor::removeListener(R2TextEditorListener* listener)
{
    listeners.remove (listener);
}

}   //  namespace r2juce

