/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

namespace r2juce {
class R2CloudManager;
}

//==============================================================================
/**
 */
class CloudDocAudioProcessor : public juce::AudioProcessor {
public:
    //==============================================================================
    CloudDocAudioProcessor();
    ~CloudDocAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
#endif

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    /**
     * @brief Gets the cloud manager instance.
     * @return A reference to the R2CloudManager.
     */
    r2juce::R2CloudManager& getCloudManager();

    // Settings state getters used by the UI
    const juce::String& getInitialPath() const;
    const juce::String& getInitialFilename() const;
    int getInitialServiceId() const;
    const juce::String& getInitialFileContent() const;

    // Settings state setters called from the UI
    void setCurrentPath(const juce::String& newPath);
    void setCurrentFilename(const juce::String& newFilename);
    void setCurrentServiceId(int newServiceId);
    void setCurrentFileContent(const juce::String& content);

private:
    //==============================================================================
    void loadSettings();
    void saveSettings();

    std::unique_ptr<r2juce::R2CloudManager> cloudManager;
    std::unique_ptr<juce::PropertiesFile> settingsFile;

    // State variables for settings
    juce::String currentPath;
    juce::String currentFilename;
    int currentServiceId;
    juce::String fileContent;

    // Keys for saving and loading settings
    static inline const juce::Identifier lastSelectedServiceKey{ "lastSelectedService" };
    static inline const juce::Identifier lastFilePathKey{ "lastFilePath" };
    static inline const juce::Identifier lastFileNameKey{ "lastFileName" };

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloudDocAudioProcessor)
};

