/*
  ==============================================================================

    R2TextEditor.h
    Created: 10 May 2022 8:36:15pm
    Author:  ring2

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "R2ScreenKeyboard.h"

namespace r2juce {

class R2TextEditor : public juce::TextEditor
{
public:
    R2TextEditor(const juce::String& componentName = juce::String(), wchar_t passwordCharacter = 0);
    
    void focusGained (FocusChangeType) override;
    void focusLost(FocusChangeType) override;
    bool keyPressed(const juce::KeyPress& key) override;
    
    void setFocusChangeCallback(std::function<void(bool)> callback) { focusChangedCallback = callback; }
    
    bool wasExitedWithEscape() const { return exitedWithEscape; }
    
    void setKeyboardParentCallback(std::function<juce::Component*()> callback) { keyboardParentCallback = callback; }

private:
    std::function<void(bool)> focusChangedCallback = nullptr;
    std::function<juce::Component*()> keyboardParentCallback = nullptr;
    std::unique_ptr<R2ScreenKeyboard> keyboard = nullptr;
    bool exitedWithEscape = false;
#if JUCE_RASPI || JUCE_RASPI_SIMULATE
    bool useScreenKeyboard = true;
#else
    bool useScreenKeyboard = false;
#endif
};

}   //  namespace r2juce
