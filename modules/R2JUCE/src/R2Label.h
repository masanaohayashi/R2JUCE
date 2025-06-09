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
    
    // 編集機能を独自実装
    void setEditable(bool editOnSingleClick, bool editOnDoubleClick = false,
                     bool lossOfFocusDiscardsChanges = false);
    
    // マウスイベント
    void mouseDoubleClick(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;  // JUCEのshowEditor()呼び出しを防ぐ
    
    // 独自編集メソッド
    void startCustomEditing();
    void stopCustomEditing(bool applyChanges);
    bool isBeingEdited() const { return isCustomEditing; }
    
    // JUCEのshowEditorを隠す
    void showEditor();
    
    // R2TextEditor設定
    void setKeyboardParentCallback(std::function<juce::Component*()> callback);
    
    // スクリーンキーボード親コンポーネント設定（R2TextEditorに転送）
    void setScreenKeyboardParent(std::function<juce::Component*()> callback);
    
    // Component overrides
    void paint(juce::Graphics& g) override;
    
    // JUCEの編集機能は使用しない
    juce::TextEditor* createEditorComponent() override;
    
    // TextEditor::Listener overrides (juce::Labelが既に継承しているのでオーバーライドのみ)
    void textEditorReturnKeyPressed(juce::TextEditor&) override;
    void textEditorEscapeKeyPressed(juce::TextEditor&) override;
    
private:
    std::function<juce::Component*()> keyboardParentCallback = nullptr;
    std::function<juce::Component*()> screenKeyboardParentCallback = nullptr;
    std::unique_ptr<R2TextEditor> customEditor;
    bool isCustomEditing = false;
    bool customEditOnSingleClick = false;
    bool customEditOnDoubleClick = false;
    bool customLossOfFocusDiscardsChanges = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(R2Label)
};

}   //  namespace r2juce
