/*
  ==============================================================================

    R2Label.h
    Created: 9 Jun 2025 12:00:00pm
    Author:  ring2

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "R2TextEditor.h"

namespace r2juce {

class R2Label : public juce::Label
{
public:
    R2Label(const juce::String& componentName = juce::String(),
            const juce::String& labelText = juce::String());
    
    void setEditable(bool editOnSingleClick, bool editOnDoubleClick = false,
                     bool lossOfFocusDiscardsChanges = false);
    
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    
    void startCustomEditing();
    void stopCustomEditing(bool applyChanges);
    bool isBeingEdited() const { return isCustomEditing; }
    
    void showEditor();
    
    void setKeyboardParentCallback(std::function<juce::Component*()> callback);
    
    void paint(juce::Graphics& g) override;
    
    juce::TextEditor* createEditorComponent() override;
    
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void textEditorEscapeKeyPressed(juce::TextEditor&) override;
    
private:
    std::function<juce::Component*()> keyboardParentCallback = nullptr;
    std::unique_ptr<R2TextEditor> customEditor;
    bool isCustomEditing = false;
    bool customEditOnSingleClick = false;
    bool customEditOnDoubleClick = false;
    bool customLossOfFocusDiscardsChanges = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2Label)
};

}   //  namespace r2juce
