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
    // JUCEの編集機能は完全に無効化
    juce::Label::setEditable(false, false, false);
}

void R2Label::setEditable(bool editOnSingleClick, bool editOnDoubleClick, bool lossOfFocusDiscardsChanges)
{
    // 独自フラグで管理（JUCEの編集機能は常に無効のまま）
    customEditOnSingleClick = editOnSingleClick;
    customEditOnDoubleClick = editOnDoubleClick;
    customLossOfFocusDiscardsChanges = lossOfFocusDiscardsChanges;
    
    // JUCEの編集機能は絶対に有効にしない
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
        juce::Component::mouseDoubleClick(e);  // Label::mouseDoubleClickは呼ばない
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
        startCustomEditing();  // JUCEのshowEditor()ではなく独自編集
    }
    else
    {
        juce::Component::mouseUp(e);  // Label::mouseUpは呼ばない
    }
}

void R2Label::startCustomEditing()
{
    if (isCustomEditing) return;
    
    // R2TextEditorを作成
    customEditor = std::make_unique<R2TextEditor>();
    customEditor->setText(getText());
    customEditor->setBorder(juce::BorderSize<int>(1));
    
    // juce::TextEditor::Listenerとして登録
    customEditor->addListener(this);
    
    // フォント設定をR2TextEditorに適用
    customEditor->setFont(getFont());
    customEditor->applyFontToAllText(getFont());  // これが重要！
    customEditor->setJustification(getJustificationType());
    
    // 親のSliderから色設定を正確に取得
    if (auto* parentSlider = findParentComponentOfClass<juce::Slider>())
    {
        // 1. textBoxTextColourId → TextEditor::textColourId
        auto textColour = parentSlider->findColour(juce::Slider::textBoxTextColourId);
        customEditor->setColour(juce::TextEditor::textColourId, textColour);
        
        // 2. textBoxBackgroundColourId → TextEditor::backgroundColourId
        auto backgroundColour = parentSlider->findColour(juce::Slider::textBoxBackgroundColourId);
        customEditor->setColour(juce::TextEditor::backgroundColourId, backgroundColour);
        
        // 3. textBoxHighlightColourId → TextEditor::highlightColourId
        auto highlightColour = parentSlider->findColour(juce::Slider::textBoxHighlightColourId);
        customEditor->setColour(juce::TextEditor::highlightColourId, highlightColour);
        // highlightedTextColourIdは設定しない（JUCEのデフォルトを使用）
        
        // 4. textBoxOutlineColourId → TextEditor::outlineColourId & focusedOutlineColourId
        auto outlineColour = parentSlider->findColour(juce::Slider::textBoxOutlineColourId);
        customEditor->setColour(juce::TextEditor::outlineColourId, outlineColour);
        customEditor->setColour(juce::TextEditor::focusedOutlineColourId, outlineColour);
    }
    else
    {
        // Sliderが見つからない場合はLabelの色を使用（フォールバック）
        customEditor->setColour(juce::TextEditor::textColourId, findColour(juce::Label::textColourId));
        customEditor->setColour(juce::TextEditor::backgroundColourId, findColour(juce::Label::backgroundColourId));
        customEditor->setColour(juce::TextEditor::outlineColourId, findColour(juce::Label::outlineColourId));
        customEditor->setColour(juce::TextEditor::focusedOutlineColourId, findColour(juce::Label::outlineColourId));
    }
    
    // キーボード親コンポーネント設定
    if (keyboardParentCallback != nullptr) {
        customEditor->setKeyboardParentCallback(keyboardParentCallback);
    }
    
    // 親コンポーネントに追加
    auto* parent = getParentComponent();
    if (parent != nullptr) {
        parent->addAndMakeVisible(customEditor.get());
        customEditor->setBounds(getBounds());
        
        // フォーカスロスト時のコールバック
        customEditor->setFocusChangeCallback([this](bool hasFocus) {
            if (!hasFocus) {
                // ESCキーで終了したかどうかで判定
                bool applyChanges = !customEditor->wasExitedWithEscape();
                stopCustomEditing(applyChanges);
            }
        });
        
        // フォーカスを与える
        customEditor->grabKeyboardFocus();
        customEditor->selectAll();
        
        // Labelを非表示にする
        setVisible(false);
        
        isCustomEditing = true;
    }
}

void R2Label::stopCustomEditing(bool applyChanges)
{
    if (!isCustomEditing) return;
    
    // テキストを適用
    if (applyChanges && customEditor != nullptr) {
        setText(customEditor->getText(), juce::sendNotification);
    }
    
    // エディタを安全に削除
    if (customEditor != nullptr) {
        // juce::TextEditor::Listenerとしての登録を解除
        customEditor->removeListener(this);
        
        // R2ScreenKeyboardとの接続を切断
        customEditor->setFocusChangeCallback(nullptr);
        
        auto* parent = customEditor->getParentComponent();
        if (parent != nullptr) {
            parent->removeChildComponent(customEditor.get());
        }
        
        // 非同期で削除（安全のため）
        auto editorToDelete = customEditor.release();
        juce::MessageManager::callAsync([editorToDelete]() {
            delete editorToDelete;
        });
    }
    
    // Labelを再表示
    setVisible(true);
    
    isCustomEditing = false;
}

void R2Label::textEditorReturnKeyPressed(juce::TextEditor&)
{
    // Enterキーが押された場合は変更を適用
    stopCustomEditing(true);
}

void R2Label::textEditorEscapeKeyPressed(juce::TextEditor&)
{
    // ESCキーが押された場合は変更を破棄
    stopCustomEditing(false);
}

void R2Label::paint(juce::Graphics& g)
{
    // 編集中は何も描画しない（R2TextEditorが表示されているため）
    if (isCustomEditing)
        return;
    
    // JUCEの標準Label描画を使用
    juce::Label::paint(g);
}

void R2Label::setKeyboardParentCallback(std::function<juce::Component*()> callback)
{
    keyboardParentCallback = callback;
}

void R2Label::showEditor()
{
    // JUCEのshowEditorが呼ばれても何もしない
    // 代わりに独自編集を開始
    startCustomEditing();
}

juce::TextEditor* R2Label::createEditorComponent()
{
    // 使用しない（JUCEの編集機能は無効化しているため）
    return nullptr;
}

}   //  namespace r2juce
