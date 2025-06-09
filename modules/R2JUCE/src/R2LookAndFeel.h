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
    
    // SliderのTextBoxにR2Labelを使用
    juce::Label* createSliderTextBox(juce::Slider& slider) override;
    
    // キーボード親コンポーネントを取得するためのコールバック
    void setKeyboardParentCallback(std::function<juce::Component*()> callback);
    
    // R2TextEditorの見た目をカスタマイズ
    void fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                  juce::TextEditor& textEditor) override;
    
    void drawTextEditorOutline(juce::Graphics& g, int width, int height,
                               juce::TextEditor& textEditor) override;
    
private:
    std::function<juce::Component*()> keyboardParentCallback = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2LookAndFeel)
};

}   //  namespace r2juce
