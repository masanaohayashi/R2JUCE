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
#include "PluginProcessor.h"
//[/Headers]



//==============================================================================
/**
                                                                    //[Comments]
    An auto-generated component, created by the Projucer.

    Describe your class and how it works here!
                                                                    //[/Comments]
*/
class AudioConfigEditor  : public juce::AudioProcessorEditor,
                           public juce::Button::Listener
{
public:
    //==============================================================================
    AudioConfigEditor (AudioConfigAudioProcessor& p);
    ~AudioConfigEditor() override;

    //==============================================================================
    //[UserMethods]     -- You can add your own custom methods in this section.
    //[/UserMethods]

    void paint (juce::Graphics& g) override;
    void resized() override;
    void buttonClicked (juce::Button* buttonThatWasClicked) override;



private:
    //[UserVariables]   -- You can add your own custom variables in this section.
    class AudioConfigManager : public r2juce::R2AudioConfigManager
    {
    public:
        AudioConfigManager(const juce::String& appName) : r2juce::R2AudioConfigManager(appName) {}

    protected:
        AudioConfig getDefaultConfig() override
        {
            AudioConfig config;

            // 音楽制作向けの初期設定
            config.outputDeviceName = "ASIO";
            config.inputDeviceName = "ASIO";
            config.sampleRate = 44100.0;
            config.bufferSize = 128;

            // マルチチャンネル入出力を有効化
            config.inputChannels.setRange(0, 2, true);   // 2チャンネル入力
            config.outputChannels.setRange(0, 2, true);  // 2チャンネル出力

            return config;
        }
    };

    AudioConfigAudioProcessor& audioProcessor;
    AudioConfigManager audioConfigManager;

    //[/UserVariables]

    //==============================================================================
    std::unique_ptr<juce::Label> labelOutputDevice;
    std::unique_ptr<juce::Label> labelOutputDeviceCaption;
    std::unique_ptr<juce::TextButton> buttonRefresh;
    std::unique_ptr<juce::Label> labelInputDevice;
    std::unique_ptr<juce::Label> labelInputDeviceCaption;
    std::unique_ptr<juce::Label> labelBufferSize;
    std::unique_ptr<juce::Label> labelBufferSizeCaption;
    std::unique_ptr<juce::Label> labelSampleRate;
    std::unique_ptr<juce::Label> labelSampleRateCaption;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioConfigEditor)
};

//[EndFile] You can add extra defines here...
//[/EndFile]

