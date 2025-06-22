/*
  ==============================================================================

    R2Label.cpp
    Created: 9 Jun 2025 12:00:00pm
    Author:  ring2

  ==============================================================================
*/

#include "R2Label.h"

namespace r2juce {

R2Label::R2Label(const juce::String& componentName, const juce::String& labelText)
    : juce::Label(componentName, labelText)
{
    juce::Label::setEditable(false, false, false);
}

void R2Label::setEditable(bool editOnSingleClick, bool editOnDoubleClick, bool lossOfFocusDiscardsChanges)
{
    customEditOnSingleClick = editOnSingleClick;
    customEditOnDoubleClick = editOnDoubleClick;
    customLossOfFocusDiscardsChanges = lossOfFocusDiscardsChanges;
    
    juce::Label::setEditable(false, false, false);
}

void R2Label::mouseDoubleClick(const juce::MouseEvent& e)
{
    if (customEditOnDoubleClick
        && isEnabled()
        && !e.mods.isPopupMenu()
        && !isCustomEditing)
    {
        startCustomEditing();
    }
    else
    {
        juce::Component::mouseDoubleClick(e);
    }
}

void R2Label::mouseUp(const juce::MouseEvent& e)
{
    if (customEditOnSingleClick
        && isEnabled()
        && contains(e.getPosition())
        && !(e.mouseWasDraggedSinceMouseDown() || e.mods.isPopupMenu())
        && !isCustomEditing)
    {
        startCustomEditing();
    }
    else
    {
        juce::Component::mouseUp(e);
    }
}

void R2Label::startCustomEditing()
{
    if (isCustomEditing) return;
    
    customEditor = std::make_unique<R2TextEditor>();
    customEditor->setText(getText());
    customEditor->setBorder(juce::BorderSize<int>(1));
    
    customEditor->addListener(this);
    
    customEditor->setFont(getFont());
    customEditor->applyFontToAllText(getFont());
    customEditor->setJustification(getJustificationType());
    
    if (auto* parentSlider = findParentComponentOfClass<juce::Slider>())
    {
        auto textColour = parentSlider->findColour(juce::Slider::textBoxTextColourId);
        customEditor->setColour(juce::TextEditor::textColourId, textColour);
        
        auto backgroundColour = parentSlider->findColour(juce::Slider::textBoxBackgroundColourId);
        customEditor->setColour(juce::TextEditor::backgroundColourId, backgroundColour);
        
        auto highlightColour = parentSlider->findColour(juce::Slider::textBoxHighlightColourId);
        customEditor->setColour(juce::TextEditor::highlightColourId, highlightColour);
        
        auto outlineColour = parentSlider->findColour(juce::Slider::textBoxOutlineColourId);
        customEditor->setColour(juce::TextEditor::outlineColourId, outlineColour);
        customEditor->setColour(juce::TextEditor::focusedOutlineColourId, outlineColour);
    }
    else
    {
        customEditor->setColour(juce::TextEditor::textColourId, findColour(juce::Label::textColourId));
        customEditor->setColour(juce::TextEditor::backgroundColourId, findColour(juce::Label::backgroundColourId));
        customEditor->setColour(juce::TextEditor::outlineColourId, findColour(juce::Label::outlineColourId));
        customEditor->setColour(juce::TextEditor::focusedOutlineColourId, findColour(juce::Label::outlineColourId));
    }
    
    if (keyboardParentCallback != nullptr) {
        customEditor->setKeyboardParentCallback(keyboardParentCallback);
    }
    
    auto* parent = getParentComponent();
    if (parent != nullptr) {
        parent->addAndMakeVisible(customEditor.get());
        customEditor->setBounds(getBounds());
        
        customEditor->setFocusChangeCallback([this](bool hasFocus) {
            if (!hasFocus) {
                bool applyChanges = !customEditor->wasExitedWithEscape();
                stopCustomEditing(applyChanges);
            }
        });
        
        customEditor->grabKeyboardFocus();
        customEditor->selectAll();
        
        setVisible(false);
        
        isCustomEditing = true;
    }
}

void R2Label::stopCustomEditing(bool applyChanges)
{
    if (!isCustomEditing) return;
    
    if (applyChanges && customEditor != nullptr) {
        setText(customEditor->getText(), juce::sendNotification);
    }
    
    if (customEditor != nullptr) {
        customEditor->removeListener(this);
        
        customEditor->setFocusChangeCallback(nullptr);
        
        auto* parent = customEditor->getParentComponent();
        if (parent != nullptr) {
            parent->removeChildComponent(customEditor.get());
        }
        
        auto editorToDelete = customEditor.release();
        juce::MessageManager::callAsync([editorToDelete]() {
            delete editorToDelete;
        });
    }
    
    setVisible(true);
    
    isCustomEditing = false;
}

void R2Label::textEditorReturnKeyPressed(juce::TextEditor&)
{
    stopCustomEditing(true);
}

void R2Label::textEditorEscapeKeyPressed(juce::TextEditor&)
{
    stopCustomEditing(false);
}

void R2Label::paint(juce::Graphics& g)
{
    if (isCustomEditing)
        return;
    
    juce::Label::paint(g);
}

void R2Label::setKeyboardParentCallback(std::function<juce::Component*()> callback)
{
    keyboardParentCallback = callback;
}

void R2Label::showEditor()
{
    startCustomEditing();
}

juce::TextEditor* R2Label::createEditorComponent()
{
    return nullptr;
}

}   //  namespace r2juce
