/*
  ==============================================================================

    R2LookAndFeel.cpp
    Created: 9 Jun 2025 12:00:00pm
    Author:  ring2

  ==============================================================================
*/

#include "R2LookAndFeel.h"

namespace r2juce {

R2LookAndFeel::R2LookAndFeel()
{
}

R2LookAndFeel::~R2LookAndFeel()
{
}

juce::Label* R2LookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    // R2Labelを作成
    auto* r2Label = new R2Label();
    
    // 編集可能に設定
    r2Label->setEditable(true, true, false);
    r2Label->setJustificationType(juce::Justification::centred);
    
    // Sliderの色設定をLabelに転送
    r2Label->setColour(juce::Label::textColourId, slider.findColour(juce::Slider::textBoxTextColourId));
    r2Label->setColour(juce::Label::backgroundColourId, slider.findColour(juce::Slider::textBoxBackgroundColourId));
    r2Label->setColour(juce::Label::outlineColourId, slider.findColour(juce::Slider::textBoxOutlineColourId));
    
    // キーボード親コンポーネントのコールバックを設定
    if (keyboardParentCallback != nullptr)
    {
        r2Label->setKeyboardParentCallback(keyboardParentCallback);
    }
    
    return r2Label;
}

void R2LookAndFeel::setKeyboardParentCallback(std::function<juce::Component*()> callback)
{
    keyboardParentCallback = callback;
}

void R2LookAndFeel::fillTextEditorBackground(juce::Graphics& g, int width, int height,
                                             juce::TextEditor& textEditor)
{
    // TextEditorに設定された背景色を使用
    g.setColour(textEditor.findColour(juce::TextEditor::backgroundColourId));
    g.fillRoundedRectangle(0, 0, (float)width, (float)height, 3.0f);
}

void R2LookAndFeel::drawTextEditorOutline(juce::Graphics& g, int width, int height,
                                          juce::TextEditor& textEditor)
{
    // TextEditorに設定されたアウトライン色を使用
    if (textEditor.hasKeyboardFocus(true))
    {
        g.setColour(textEditor.findColour(juce::TextEditor::focusedOutlineColourId));
        g.drawRoundedRectangle(0, 0, (float)width, (float)height, 3.0f, 2.0f);
    }
    else
    {
        g.setColour(textEditor.findColour(juce::TextEditor::outlineColourId));
        g.drawRoundedRectangle(0, 0, (float)width, (float)height, 3.0f, 1.0f);
    }
}

}   //  namespace r2juce
