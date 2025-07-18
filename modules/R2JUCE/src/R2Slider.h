/*
  ==============================================================================

    R2Slider.h
    Created: 9 Jun 2025 12:00:00pm
    Author:  ring2

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace r2juce {

class R2LookAndFeel;

class R2Slider : public juce::Slider
{
public:
    R2Slider(const juce::String& componentName = juce::String());
    ~R2Slider() override;
    
    void setKeyboardParentCallback(std::function<juce::Component*()> callback);
    
    void setFocusChangeCallback(std::function<void(bool)> callback);
    
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
private:
    std::unique_ptr<R2LookAndFeel> r2LookAndFeel;
    std::function<juce::Component*()> keyboardParentCallback = nullptr;
    std::function<void(bool)> focusChangeCallback = nullptr;
    
    void updateLookAndFeel();
    void cancelEditing();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2Slider)
};

}   //  namespace r2juce
