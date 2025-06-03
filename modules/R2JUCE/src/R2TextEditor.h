/*
  ==============================================================================

    R2TextEditor.h
    Created: 10 May 2022 8:36:15pm
    Author:  ring2

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <list>
#include "R2ScreenKeyboard.h"

class R2TextEditor;
class R2TextEditorListener
{
public:
    virtual ~R2TextEditorListener() {}
    virtual void onR2TextEditorFocusGained(R2TextEditor* editor) {}
    virtual void onR2TextEditorFocusLost(R2TextEditor* editor) {}
};

class R2TextEditor : public juce::TextEditor
{
public:
    R2TextEditor(const juce::String& componentName = juce::String(), wchar_t passwordCharacter = 0);

    void focusGained (FocusChangeType) override;
    void focusLost(FocusChangeType) override;

    void setFocusChangeCallback(std::function<void(bool)> callback) { focusChangedCallback = callback; }

    void addListener (R2TextEditorListener* listener);
    void removeListener (R2TextEditorListener* listener);
    
    std::function<juce::Component*()> getKeyboardParent = nullptr;
    
private:
    std::list<R2TextEditorListener*> listeners;
    std::function<void(bool)> focusChangedCallback = nullptr;
    std::unique_ptr<R2ScreenKeyboard> keyboard = nullptr;
};
