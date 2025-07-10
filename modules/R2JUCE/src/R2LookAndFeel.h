/*
  ==============================================================================

    R2LookAndFeel.h
    Created: 9 Jun 2025 12:00:00pm
    Author:  ring2

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "R2Label.h"

namespace r2juce {

class R2LookAndFeel : public juce::LookAndFeel_V4
{
public:
    R2LookAndFeel();
    ~R2LookAndFeel() override;
    
    juce::Label* createSliderTextBox(juce::Slider& slider) override;
    
    void setKeyboardParentCallback(std::function<juce::Component*()> callback);
    
    void fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                  juce::TextEditor& textEditor) override;
    
    void drawTextEditorOutline(juce::Graphics& g, int width, int height,
                               juce::TextEditor& textEditor) override;
    

    //  Workaround to make ComboBox clickable on Mac/iOS for AUv3
    juce::Component* getParentComponentForMenuOptions (const juce::PopupMenu::Options& options) override
    {
#if JUCE_IOS || JUCE_MAC
        if (juce::PluginHostType::getPluginLoadedAs() == juce::AudioProcessor::wrapperType_AudioUnitv3)
        {
            if (options.getParentComponent() == nullptr && options.getTargetComponent() != nullptr)
                //return options.getTargetComponent()->getTopLevelComponent();
                return options.getTargetComponent()->getParentComponent();
        }
#endif
        
        return juce::LookAndFeel_V4::getParentComponentForMenuOptions (options);
    }

private:
    std::function<juce::Component*()> keyboardParentCallback = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2LookAndFeel)
};

}   //  namespace r2juce
