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
#include <functional>
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class MainComponent  : public juce::Component,
                       public juce::ComboBox::Listener,
                       public juce::Button::Listener
{
public:
    //==============================================================================
    MainComponent (CloudDocAudioProcessor& p);
    ~MainComponent() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void comboBoxChanged (juce::ComboBox* comboBoxThatHasChanged) override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
  // This is the new central method for updating the UI based on the manager's
  // state.
  void updateUiForState(const r2juce::R2CloudManager::AppState& state);

  void loadFromFile(bool showMessages = true);
  void saveToFile();
  void showMessage(const juce::String& title, const juce::String& message);
  void handleFileDroppedInArea(const juce::String& filePath,
                               const juce::MemoryBlock& fileContent);

  // UI management
  void showAuthUI();
  void hideAuthUI();

  // Reference to the processor that owns this component
  CloudDocAudioProcessor& audioProcessor;

  // Reference to the cloud manager owned by the AudioProcessor.
  r2juce::R2CloudManager& cloudManager;

  std::unique_ptr<r2juce::R2CloudAuthComponent>
      authComponent;  // Auth UI is now owned by MainComponent
  r2juce::R2AlertComponent* progressAlert = nullptr;

  bool initialLoadAttempted = false;

  class DropArea : public juce::Component,
                   public juce::FileDragAndDropTarget {
   public:
    /**
     * @brief Constructs a DropArea.
     */
    DropArea() : isHighlighted(false) {}

    void paint(juce::Graphics& g) override {
      int x = (getWidth() / 2) - (72 / 2), y = (getHeight() / 2) - (24 / 2),
          width = 72, height = 24;
      juce::String text(TRANS("File"));
      juce::Colour fillColour = juce::Colours::white;
      g.setColour(fillColour);
      g.setFont(juce::Font(juce::FontOptions(15.00f, juce::Font::plain)));
      g.drawText(text, x, y, width, height, juce::Justification::centred, true);

      juce::Colour strokeColour = juce::Colours::white;
      g.setColour(strokeColour);
      g.drawRect(0, 0, getWidth(), getHeight(), 1);

      // Draw highlight if active
      if (isHighlighted) {
        g.setColour(juce::Colours::cyan.withAlpha(0.5f));
        g.fillRect(getLocalBounds());
      }
    }

    //======================================================================
    // juce::FileDragAndDropTarget overrides
    bool isInterestedInFileDrag(const juce::StringArray& files) override;
    void fileDragEnter(const juce::StringArray& files, int x, int y) override;
    void fileDragMove(const juce::StringArray& files, int x, int y) override;
    void filesDropped(const juce::StringArray& files, int x, int y) override;
    void fileDragExit(const juce::StringArray& files) override;

    /** @brief Callback for when a file is successfully dropped and read.
     * @param filePath The full path of the dropped file.
     * @param fileContent The content of the dropped file as a MemoryBlock.
     */
    std::function<void(const juce::String& filePath,
                       const juce::MemoryBlock& fileContent)>
        onFileDropped;

   private:
    bool isHighlighted;  // To manage visual feedback for drag and drop
  };

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Label> labelCloudService;
    std::unique_ptr<juce::ComboBox> comboService;
    std::unique_ptr<r2juce::R2TextEditor> textEditorData;
    std::unique_ptr<juce::TextButton> textButtonLoad;
    std::unique_ptr<juce::TextButton> textButtonSave;
    std::unique_ptr<juce::Label> labelFilename;
    std::unique_ptr<r2juce::R2TextEditor> textEditorFilename;
    std::unique_ptr<juce::TextButton> textButtonSignOut;
    std::unique_ptr<juce::Label> labelPath;
    std::unique_ptr<r2juce::R2TextEditor> textEditorPath;
    std::unique_ptr<juce::Label> labelData;
    std::unique_ptr<DropArea> dropArea;
    std::unique_ptr<juce::Label> labelStatus;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

