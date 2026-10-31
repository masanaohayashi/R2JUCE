#include "R2AlertComponent.h"

namespace r2juce {

float R2AlertComponent::globalContentScale = 1.0f;

//==============================================================================
R2AlertComponent::R2AlertComponent (juce::Component* parent, const juce::String& title, const juce::String& message, const juce::StringArray& buttonLabels, bool showProgressBar, std::function<void(int)> callback)
    : parentComponent (parent), onResult (callback), isProgressBarVisible (showProgressBar)
{

    labelTitle.reset (new juce::Label (juce::String(),
                                       TRANS ("(Placeholder)")));
    addAndMakeVisible (labelTitle.get());
    labelTitle->setFont (juce::Font (juce::FontOptions (20.00f, juce::Font::plain)));
    labelTitle->setJustificationType (juce::Justification::centredLeft);
    labelTitle->setEditable (false, false, false);
    labelTitle->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelTitle->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    labelMessage.reset (new juce::Label (juce::String(),
                                         TRANS ("label text\n"
                                         "asdasd\n")));
    addAndMakeVisible (labelMessage.get());
    labelMessage->setFont (juce::Font (juce::FontOptions (15.00f, juce::Font::plain)));
    labelMessage->setJustificationType (juce::Justification::topLeft);
    labelMessage->setEditable (false, false, false);
    labelMessage->setColour (juce::TextEditor::textColourId, juce::Colours::black);
    labelMessage->setColour (juce::TextEditor::backgroundColourId, juce::Colour (0x00000000));

    button1.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (button1.get());
    button1->setButtonText (TRANS ("1"));
    button1->addListener (this);
    button1->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff505050));

    button2.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (button2.get());
    button2->setButtonText (TRANS ("2"));
    button2->addListener (this);
    button2->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff505050));

    button3.reset (new juce::TextButton (juce::String()));
    addAndMakeVisible (button3.get());
    button3->setButtonText (TRANS ("3"));
    button3->addListener (this);
    button3->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff505050));

    progressBar.reset (new juce::ProgressBar (currentProgress, juce::ProgressBar::Style::circular));
    addAndMakeVisible (progressBar.get());


    jassert (parent != nullptr);
    parent->addAndMakeVisible (this);

    labelTitle->setText (title, juce::dontSendNotification);
    labelMessage->setText (message, juce::dontSendNotification);
    numButtons = buttonLabels.size();
    for (int i = 0; i < std::min(3, numButtons); i++) {
        juce::TextButton* button = nullptr;
        if (i == 0) button = button1.get();
        else if (i == 1) button = button2.get();
        else if (i == 2) button = button3.get();

        button->setButtonText (buttonLabels[i]);
    }

    if (isProgressBarVisible) {
        progressBar->setColour (juce::ProgressBar::foregroundColourId, juce::Colours::transparentBlack);
        progressBar->setColour (juce::ProgressBar::foregroundColourId, juce::Colours::white);
    }
    else {
        progressBar->setVisible(false);
    }

    setSize (parent->getWidth(), parent->getHeight());
    setTopLeftPosition(0, 0);
    setWantsKeyboardFocus(true);
    grabKeyboardFocus();
    selectedButtonIndex = 1; // Default to first button
    updateButtonFocus();
    applyContentScaleIfNeeded();
    parentComponent->addComponentListener(this);
}

R2AlertComponent::~R2AlertComponent()
{
    if (parentComponent != nullptr)
        parentComponent->removeComponentListener (this);

    labelTitle = nullptr;
    labelMessage = nullptr;
    button1 = nullptr;
    button2 = nullptr;
    button3 = nullptr;
    progressBar = nullptr;


    progressBar = nullptr;
}

//==============================================================================
void R2AlertComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xc0000000));

    {
        int x = (getWidth() / 2) - (proportionOfWidth (0.9155f) / 2), y = (getHeight() / 2) - (proportionOfHeight (0.5250f) / 2), width = proportionOfWidth (0.9155f), height = proportionOfHeight (0.5250f);
        juce::Colour fillColour = juce::Colours::transparentBlack;
        g.setColour (fillColour);
        g.fillRect (x, y, width, height);
    }

    g.fillAll (juce::Colour (0xc0000000));

    auto panelBounds = contentBounds;
    if (panelBounds.isEmpty())
    {
        const auto scale = getContentScale();
        const auto fallbackBounds = getLocalBounds().reduced (juce::jmin (16, getWidth() / 8),
                                                              juce::jmin (16, getHeight() / 8));
        const auto panelWidth = juce::jmin (juce::roundToInt (520.0f * scale), fallbackBounds.getWidth());
        const auto panelHeight = juce::jmin (juce::roundToInt (220.0f * scale), fallbackBounds.getHeight());
        panelBounds = juce::Rectangle<int> (panelWidth, panelHeight).withCentre (fallbackBounds.getCentre());
    }

    g.setColour (juce::Colour (0xff323e44));
    g.fillRect (panelBounds);
}

void R2AlertComponent::resized()
{
    labelTitle->setBounds ((getWidth() / 2) - (proportionOfWidth (0.8592f) / 2), proportionOfHeight (0.2500f), proportionOfWidth (0.8592f), 32);
    labelMessage->setBounds ((getWidth() / 2) - (proportionOfWidth (0.8592f) / 2), proportionOfHeight (0.5000f) - (proportionOfHeight (0.2500f) / 2), proportionOfWidth (0.8592f), proportionOfHeight (0.2500f));
    button1->setBounds (proportionOfWidth (0.2465f) - (proportionOfWidth (0.2113f) / 2), (getHeight() / 2) + 60 - (24 / 2), proportionOfWidth (0.2113f), 24);
    button2->setBounds (proportionOfWidth (0.5000f) - (proportionOfWidth (0.2113f) / 2), (getHeight() / 2) + 60 - (24 / 2), proportionOfWidth (0.2113f), 24);
    button3->setBounds (proportionOfWidth (0.7535f) - (proportionOfWidth (0.2113f) / 2), (getHeight() / 2) + 60 - (24 / 2), proportionOfWidth (0.2113f), 24);
    progressBar->setBounds (proportionOfWidth (0.5000f) - (32 / 2), proportionOfHeight (0.5000f), 32, 32);

    if (numButtons == 1)
    {
        button1->setBounds ((getWidth() / 2) - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
        button2->setVisible (false);
        button3->setVisible (false);
    }
    else if (numButtons == 2)
    {
        button1->setBounds ((getWidth() / 2) + -80 - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
        button2->setBounds ((getWidth() / 2) + 80 - (120 / 2), (getHeight() / 2) + 60 - (24 / 2), 120, 24);
        button3->setVisible (false);
    }

    layoutComponentsWithScale (getContentScale());
}

void R2AlertComponent::buttonClicked (juce::Button* buttonThatWasClicked)
{
    if (buttonThatWasClicked == button1.get())
    {
        buttonClicked (1);
    }
    else if (buttonThatWasClicked == button2.get())
    {
        buttonClicked (2);
    }
    else if (buttonThatWasClicked == button3.get())
    {
        buttonClicked (3);
    }
}



bool R2AlertComponent::keyPressed (const juce::KeyPress& key)
{
    if ((key == juce::KeyPress::spaceKey) || (key == juce::KeyPress::returnKey)) // Space/Enter
    {
        buttonClicked (selectedButtonIndex);
        return true;
    }
    if (key == juce::KeyPress::escapeKey) // Esc
    {
        buttonClicked (numButtons); // Cancel, No, etc.
        return true;
    }
    if (key == juce::KeyPress::leftKey) // ←キー
    {
        selectedButtonIndex -= 1;
        if (selectedButtonIndex < 1) selectedButtonIndex = numButtons;
        updateButtonFocus();
        return true;
    }
    if (key == juce::KeyPress::rightKey) // →キー
    {
        selectedButtonIndex += 1;
        if (selectedButtonIndex > numButtons) selectedButtonIndex = 1;
        updateButtonFocus();
        return true;
    }

    return true;
}

void R2AlertComponent::buttonClicked (int index)
{
    juce::Component::SafePointer<R2AlertComponent> safeThis (this);

    auto callback = onResult;
    onResult = nullptr;
    
    if (callback)
        callback (index);

    if (safeThis != nullptr)
        safeThis->close();
}

void R2AlertComponent::updateButtonFocus()
{
    auto dullBlue = juce::Colour(60, 105, 170); // くすんだ青
    auto normalGrey = juce::Colours::grey;

    button1->setColour (juce::TextButton::buttonColourId, selectedButtonIndex == 1 ? dullBlue : normalGrey);
    button2->setColour (juce::TextButton::buttonColourId, selectedButtonIndex == 2 ? dullBlue : normalGrey);
    button3->setColour (juce::TextButton::buttonColourId, selectedButtonIndex == 3 ? dullBlue : normalGrey);
    repaint();
}

void R2AlertComponent::forOK (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback)
{
    new R2AlertComponent (parent, title, message, { TRANS("OK") }, false, callback);
}

void R2AlertComponent::forYesNo (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback)
{
    new R2AlertComponent (parent, title, message, { TRANS("Yes"), TRANS("No") }, false, callback);
}

void R2AlertComponent::forYesNoCancel (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback)
{
    new R2AlertComponent (parent, title, message, { TRANS("Yes"), TRANS("No"), TRANS("Cancel") }, false, callback);
}

R2AlertComponent* R2AlertComponent::forProgress (juce::Component* parent, const juce::String& title, const juce::String& message, double initialProgress, std::function<void(int)> callback)
{
    auto* alert = new R2AlertComponent (parent, title, message, { TRANS("Cancel") }, true, callback);
    alert->selectedButtonIndex = 1; // For progress bar, only one button "Cancel" is available
    alert->updateButtonFocus();
    alert->setProgress (initialProgress); // Call R2AlertComponent::setProgress()
    return alert;
}

void R2AlertComponent::setProgress (double newProgress)
{
    if (progressBar != nullptr)
    {
        currentProgress = newProgress; // double 変数を直接更新する
    }
}

void R2AlertComponent::close() // メソッド名をcloseAlert()からclose()に変更
{
    if (isClosing)
        return;

    isClosing = true;
    juce::Component::SafePointer<R2AlertComponent> safeThis (this);

    juce::MessageManager::callAsync([safeThis]()
    {
        if (safeThis == nullptr)
            return;

        if (auto* parent = safeThis->getParentComponent())
            parent->removeChildComponent(safeThis.getComponent());

        delete safeThis.getComponent();
    });
}

void R2AlertComponent::setGlobalContentScale (float scaleToApply)
{
    globalContentScale = juce::jlimit (1.0f, 4.0f, scaleToApply);
}

float R2AlertComponent::getGlobalContentScale()
{
    return globalContentScale;
}

float R2AlertComponent::getContentScale() const
{
    return globalContentScale;
}

void R2AlertComponent::applyContentScaleIfNeeded()
{
    const auto scale = getContentScale();
    if (scale <= 1.0f)
        return;

    if (labelTitle != nullptr)
        labelTitle->setFont (juce::Font (juce::FontOptions (20.0f * scale, juce::Font::plain)));

    if (labelMessage != nullptr)
        labelMessage->setFont (juce::Font (juce::FontOptions (15.0f * scale, juce::Font::plain)));
}

void R2AlertComponent::layoutComponentsWithScale (float scale)
{
    {
        const auto safeScale = juce::jmax (1.0f, scale);
        const auto outerMargin = juce::roundToInt (16.0f * safeScale);
        const auto availableBounds = getLocalBounds().reduced (juce::jmin (outerMargin, getWidth() / 8),
                                                               juce::jmin (outerMargin, getHeight() / 8));

        if (availableBounds.isEmpty())
        {
            contentBounds = {};
            return;
        }

        const auto targetWidth = juce::roundToInt (520.0f * safeScale);
        const auto targetHeight = juce::roundToInt ((isProgressBarVisible ? 250.0f : 220.0f) * safeScale);
        const auto panelWidth = juce::jmin (targetWidth, availableBounds.getWidth());
        const auto panelHeight = juce::jmin (targetHeight, availableBounds.getHeight());

        contentBounds = juce::Rectangle<int> (panelWidth, panelHeight).withCentre (availableBounds.getCentre());

        auto content = contentBounds.reduced (juce::jlimit (12, 28, juce::roundToInt (24.0f * safeScale)),
                                              juce::jlimit (10, 24, juce::roundToInt (20.0f * safeScale)));

        const auto titleHeight = juce::jlimit (24, 40, juce::roundToInt (32.0f * safeScale));
        const auto gap = juce::jlimit (6, 14, juce::roundToInt (10.0f * safeScale));
        const auto buttonHeight = juce::jlimit (24, 34, juce::roundToInt (28.0f * safeScale));
        const auto progressHeight = isProgressBarVisible ? juce::jlimit (12, 24, juce::roundToInt (16.0f * safeScale)) : 0;
        const auto visibleButtonCount = juce::jlimit (1, 3, numButtons);
        const auto buttonRows = (visibleButtonCount > 1 && content.getWidth() < 260) ? visibleButtonCount : 1;
        const auto buttonAreaHeight = (buttonRows * buttonHeight) + ((buttonRows - 1) * gap);
        const auto progressAreaHeight = isProgressBarVisible ? progressHeight + gap : 0;
        const auto reservedBottomHeight = progressAreaHeight + buttonAreaHeight;

        if (labelTitle != nullptr)
            labelTitle->setBounds (content.removeFromTop (juce::jmin (titleHeight, content.getHeight())));

        content.removeFromTop (juce::jmin (gap, content.getHeight()));

        auto bottomArea = content.removeFromBottom (juce::jmin (reservedBottomHeight, content.getHeight()));

        if (labelMessage != nullptr)
            labelMessage->setBounds (content);

        if (progressBar != nullptr)
        {
            progressBar->setVisible (isProgressBarVisible);

            if (isProgressBarVisible)
            {
                auto progressArea = bottomArea.removeFromTop (juce::jmin (progressHeight, bottomArea.getHeight()));
                progressBar->setBounds (progressArea);
                bottomArea.removeFromTop (juce::jmin (gap, bottomArea.getHeight()));
            }
        }

        juce::TextButton* buttons[] = { button1.get(), button2.get(), button3.get() };

        for (int i = 0; i < 3; ++i)
            if (buttons[i] != nullptr)
                buttons[i]->setVisible (i < visibleButtonCount);

        if (visibleButtonCount == 1)
        {
            if (button1 != nullptr)
            {
                auto row = bottomArea.removeFromTop (juce::jmin (buttonHeight, bottomArea.getHeight()));
                button1->setBounds (row.withSizeKeepingCentre (juce::jmin (120, row.getWidth()), row.getHeight()));
            }
        }
        else if (buttonRows > 1)
        {
            for (int i = 0; i < visibleButtonCount; ++i)
            {
                auto row = bottomArea.removeFromTop (juce::jmin (buttonHeight, bottomArea.getHeight()));
                if (buttons[i] != nullptr)
                    buttons[i]->setBounds (row);

                bottomArea.removeFromTop (juce::jmin (gap, bottomArea.getHeight()));
            }
        }
        else
        {
            const auto totalGap = gap * (visibleButtonCount - 1);
            const auto buttonWidth = (bottomArea.getWidth() - totalGap) / visibleButtonCount;
            auto row = bottomArea.removeFromTop (juce::jmin (buttonHeight, bottomArea.getHeight()));

            for (int i = 0; i < visibleButtonCount; ++i)
            {
                if (buttons[i] != nullptr)
                    buttons[i]->setBounds (row.removeFromLeft (buttonWidth));

                row.removeFromLeft (gap);
            }
        }
    }

    return;

    const auto centreX = getWidth() / 2;
    const auto centreY = getHeight() / 2;

    const auto panelWidth = juce::roundToInt (520.0f * scale);
    const auto panelHeight = juce::roundToInt (168.0f * scale);
    contentBounds = { centreX - panelWidth / 2,
                      centreY - panelHeight / 2,
                      panelWidth,
                      panelHeight };

    const auto titleWidth = juce::roundToInt (488.0f * scale);
    const auto titleHeight = juce::roundToInt (32.0f * scale);
    if (labelTitle != nullptr)
        labelTitle->setBounds (centreX - titleWidth / 2,
                               centreY - juce::roundToInt (80.0f * scale),
                               titleWidth,
                               titleHeight);

    const auto messageHeight = juce::roundToInt (80.0f * scale);
    if (labelMessage != nullptr)
        labelMessage->setBounds (centreX - titleWidth / 2,
                                 centreY - messageHeight / 2,
                                 titleWidth,
                                 messageHeight);

    const auto buttonWidth = juce::roundToInt (120.0f * scale);
    const auto buttonHeight = juce::roundToInt (24.0f * scale);
    const auto buttonBaseY = centreY + juce::roundToInt (60.0f * scale) - buttonHeight / 2;

    if (button1 != nullptr)
        button1->setVisible (true);
    if (button2 != nullptr)
        button2->setVisible (numButtons >= 2);
    if (button3 != nullptr)
        button3->setVisible (numButtons >= 3);

    const auto setButtonBounds = [=](juce::TextButton* button, float offsetX)
    {
        if (button == nullptr)
            return;

        button->setBounds (centreX + juce::roundToInt (offsetX * scale) - buttonWidth / 2,
                            buttonBaseY,
                            buttonWidth,
                            buttonHeight);
    };

    if (numButtons == 1)
    {
        setButtonBounds (button1.get(), 0.0f);
    }
    else if (numButtons == 2)
    {
        setButtonBounds (button1.get(), -80.0f);
        setButtonBounds (button2.get(), 80.0f);
    }
    else
    {
        setButtonBounds (button1.get(), -144.0f);
        setButtonBounds (button2.get(), 0.0f);
        setButtonBounds (button3.get(), 144.0f);
    }

    if (button2 != nullptr && numButtons < 2)
        button2->setVisible (false);

    if (button3 != nullptr && numButtons < 3)
        button3->setVisible (false);

    if (isProgressBarVisible && progressBar != nullptr)
    {
        const auto progressHeight = juce::roundToInt (15.0f * scale);
        progressBar->setBounds (centreX - titleWidth / 2,
                                centreY + juce::roundToInt (20.0f * scale),
                                titleWidth,
                                progressHeight);
    }
}

}   //  namespace r2juce
