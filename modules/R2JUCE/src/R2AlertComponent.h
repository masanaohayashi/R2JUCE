/*
  ==============================================================================

  This is an automatically generated GUI class created by the Projucer!

  Be careful when adding custom code to these files, as only the code within
  the "//[xyz]" and "//[/xyz]" sections will be retained when the file is loaded
  and re-saved.

  Created with Projucer version: 7.0.12

  ------------------------------------------------------------------------------

  The Projucer is part of the JUCE library.
  Copyright (c) 2020 - Raw Material Software Limited.

  ==============================================================================
*/

#pragma once

//[Headers]     -- You can add your own extra header files here --
#include <JuceHeader.h>
namespace r2juce {
//[/Headers]


//==============================================================================
/**
 //[Comments]
  @brief A component that displays a modal alert dialog.

  This class is displayed as an overlay on top of a parent component
  to present information and prompt the user for a response.
  It can have a title, a message, and 1 to 3 custom buttons.
  It also supports keyboard navigation (arrow keys, Enter, Esc).
  The result is returned via a callback function passed in the constructor.
 //[/Comments]
 */
class R2AlertComponent  : public juce::Component,
                          public juce::Button::Listener
{
public:
    //==============================================================================
    /**
     @brief Constructs an R2AlertComponent.
     @param parent       The parent component on which to display this alert. Must not be nullptr.
     @param title        The title displayed at the top of the dialog.
     @param message      The main message text to display in the dialog.
     @param buttonLabels An array of strings for the button labels. Must contain 1 to 3 elements.
     @param callback     The function to call when a button is clicked. It receives the 1-based
                         index of the button that was pressed.
    */
    R2AlertComponent (juce::Component* parent, const juce::String& title, const juce::String& message, const juce::StringArray& buttonLabels, std::function<void(int)> callback);

    /** @brief Destructor. */
    ~R2AlertComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.

    /**
     @brief Handles keyboard input events.
     @details
     - Left/Right arrow keys: Move focus between buttons.
     - Enter/Space key: Clicks the currently focused button.
     - Escape key: Triggers the cancel action, which is equivalent to clicking the
                   last button (usually "Cancel" or "No").
     @param key A KeyPress object containing information about the key that was pressed.
     @return Returns true if the event was handled, false otherwise.
    */
    bool keyPressed (const juce::KeyPress& key) override;

    /**
     @brief A static helper function to easily display a simple alert dialog with a single "OK" button.
     @param parent   The parent component for the alert.
     @param title    The dialog's title.
     @param message  The dialog's message.
     @param callback An optional callback function to be executed when the button is clicked.
    */
    static void forOK (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback=nullptr);

    /**
     @brief A static helper function to display an alert dialog with "Yes" and "No" buttons.
     @param parent   The parent component for the alert.
     @param title    The dialog's title.
     @param message  The dialog's message.
     @param callback An optional callback function. Receives 1 for "Yes" and 2 for "No".
    */
    static void forYesNo (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback=nullptr);

    /**
     @brief A static helper function to display an alert dialog with "Yes", "No", and "Cancel" buttons.
     @param parent   The parent component for the alert.
     @param title    The dialog's title.
     @param message  The dialog's message.
     @param callback An optional callback function. Receives 1 for "Yes", 2 for "No", and 3 for "Cancel".
    */
    static void forYesNoCancel (juce::Component* parent, const juce::String& title, const juce::String& message, std::function<void(int)> callback=nullptr);
    //[/UserMethods]

    /** @brief Paints the component. (Overridden from juce::Component). */
    void paint (juce::Graphics& g) override;

    /** @brief Adjusts the layout of child components when the component's size is changed. (Overridden from juce::Component). */
    void resized() override;

    /** @brief Handles button click events. (Overridden from juce::Button::Listener). */
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.

    /**
     @brief Handles the internal logic for a button click, invokes the callback, and deletes this component.
     @param index The 1-based index of the clicked button.
    */
    void buttonClicked (int index);

    /** @brief Updates the appearance of the buttons to visually indicate which one is currently selected/focused. */
    void updateButtonFocus();

    juce::Component* parentComponent;
    std::function<void(int)> onResult;
    int numButtons;
    int selectedButtonIndex;
    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Label> labelTitle;
    std::unique_ptr<juce::Label> labelMessage;
    std::unique_ptr<juce::TextButton> button1;
    std::unique_ptr<juce::TextButton> button2;
    std::unique_ptr<juce::TextButton> button3;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2AlertComponent)
};

//[EndFile] You can add extra defines here...
}   //  namespace r2juce
//[/EndFile]
