/*
  ==============================================================================

    R2Slider.cpp
    Created: 9 Jun 2025 12:00:00pm
    Author:  ring2

  ==============================================================================
*/

#include "R2Slider.h"
#include "R2LookAndFeel.h"
#include "R2Label.h"

namespace r2juce {

R2Slider::R2Slider(const juce::String& componentName)
    : juce::Slider(componentName)
{
    r2LookAndFeel = std::make_unique<R2LookAndFeel>();
    
    keyboardParentCallback = [this]() -> juce::Component* {
        return this->getParentComponent();
    };
    
    updateLookAndFeel();
    
    setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 30);
}

R2Slider::~R2Slider()
{
    setLookAndFeel(nullptr);
}

void R2Slider::setKeyboardParentCallback(std::function<juce::Component*()> callback)
{
    keyboardParentCallback = callback;
    updateLookAndFeel();
    
    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        if (auto* r2Label = dynamic_cast<R2Label*>(getChildComponent(i)))
        {
            r2Label->setKeyboardParentCallback(callback);
        }
    }
}

void R2Slider::setFocusChangeCallback(std::function<void(bool)> callback)
{
    focusChangeCallback = callback;
}

void R2Slider::cancelEditing()
{
    for (int i = 0; i < getNumChildComponents(); ++i)
    {
        if (auto* r2Label = dynamic_cast<R2Label*>(getChildComponent(i)))
        {
            if (r2Label->isBeingEdited())
            {
                r2Label->stopCustomEditing(false);
                break;
            }
        }
    }
}

void R2Slider::mouseDown(const juce::MouseEvent& e)
{
    cancelEditing();
    
    juce::Slider::mouseDown(e);
}

void R2Slider::mouseDrag(const juce::MouseEvent& e)
{
    cancelEditing();
    
    juce::Slider::mouseDrag(e);
}

void R2Slider::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel)
{
    cancelEditing();
    
    juce::Slider::mouseWheelMove(e, wheel);
}

void R2Slider::updateLookAndFeel()
{
    if (r2LookAndFeel != nullptr && keyboardParentCallback != nullptr)
    {
        r2LookAndFeel->setKeyboardParentCallback(keyboardParentCallback);
        setLookAndFeel(r2LookAndFeel.get());
    }
}

}   //  namespace r2juce
