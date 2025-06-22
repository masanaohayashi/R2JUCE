#pragma once

#include <JuceHeader.h>

namespace r2juce {

class R2AudioConfigManager
{
public:
    struct AudioConfig
    {
        juce::String outputDeviceName;
        juce::String inputDeviceName;
        double sampleRate = 44100.0;
        int bufferSize = 512;
        juce::BigInteger inputChannels;
        juce::BigInteger outputChannels;
    };

    R2AudioConfigManager(const juce::String& appName)
        : configFile(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                        .getChildFile(appName)
                        .getChildFile("audio_config.json"))
    {
        configFile.getParentDirectory().createDirectory();
    }

    virtual ~R2AudioConfigManager() = default;

    AudioConfig loadConfig()
    {
        if (configFile.existsAsFile())
        {
            try
            {
                juce::String jsonText = configFile.loadFileAsString();
                juce::var json = juce::JSON::parse(jsonText);
                
                if (json.isObject())
                {
                    AudioConfig config;
                    config.outputDeviceName = json.getProperty("outputDevice", "").toString();
                    config.inputDeviceName = json.getProperty("inputDevice", "").toString();
                    config.sampleRate = (double)json.getProperty("sampleRate", 44100.0);
                    config.bufferSize = (int)json.getProperty("bufferSize", 512);
                    
                    juce::String inputChannelsStr = json.getProperty("inputChannels", "").toString();
                    juce::String outputChannelsStr = json.getProperty("outputChannels", "").toString();
                    
                    if (inputChannelsStr.isNotEmpty())
                        config.inputChannels.parseString(inputChannelsStr, 2);
                    if (outputChannelsStr.isNotEmpty())
                        config.outputChannels.parseString(outputChannelsStr, 2);
                    
                    return config;
                }
            }
            catch (...)
            {
                DBG("Failed to load settings file.Default values will be used.");
            }
        }
        else
        {
            DBG("Settings file not found. Default values will be used.");
        }
        
        return getDefaultConfig();
    }

    bool saveConfig(const AudioConfig& config)
    {
        try
        {
            juce::DynamicObject::Ptr jsonObject = new juce::DynamicObject();
            
            jsonObject->setProperty("outputDevice", config.outputDeviceName);
            jsonObject->setProperty("inputDevice", config.inputDeviceName);
            jsonObject->setProperty("sampleRate", config.sampleRate);
            jsonObject->setProperty("bufferSize", config.bufferSize);
            jsonObject->setProperty("inputChannels", config.inputChannels.toString(2));
            jsonObject->setProperty("outputChannels", config.outputChannels.toString(2));
            
            jsonObject->setProperty("lastSaved", juce::Time::getCurrentTime().toString(true, true));
            
            juce::String jsonText = juce::JSON::toString(juce::var(jsonObject.get()), true);
            
            return configFile.replaceWithText(jsonText);
        }
        catch (...)
        {
            DBG("Failed to save configuration file.");
            return false;
        }
    }

    AudioConfig getCurrentConfig(juce::AudioDeviceManager& deviceManager)
    {
        AudioConfig config;
        
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        deviceManager.getAudioDeviceSetup(setup);
        
        config.outputDeviceName = setup.outputDeviceName;
        config.inputDeviceName = setup.inputDeviceName;
        config.sampleRate = setup.sampleRate;
        config.bufferSize = setup.bufferSize;
        config.inputChannels = setup.inputChannels;
        config.outputChannels = setup.outputChannels;
        
        return config;
    }

    juce::AudioDeviceManager::AudioDeviceSetup configToSetup(const AudioConfig& config)
    {
        juce::AudioDeviceManager::AudioDeviceSetup setup;
        
        setup.outputDeviceName = config.outputDeviceName;
        setup.inputDeviceName = config.inputDeviceName;
        setup.sampleRate = config.sampleRate;
        setup.bufferSize = config.bufferSize;
        setup.inputChannels = config.inputChannels;
        setup.outputChannels = config.outputChannels;
        
        return setup;
    }

    juce::File getConfigFile() const { return configFile; }

    bool configFileExists() const { return configFile.existsAsFile(); }

protected:
    virtual AudioConfig getDefaultConfig()
    {
        AudioConfig defaultConfig;
        
        defaultConfig.outputDeviceName = "";
        defaultConfig.inputDeviceName = "";
        defaultConfig.sampleRate = 44100.0;
        defaultConfig.bufferSize = 512;
        
        defaultConfig.inputChannels.setRange(0, 2, true);
        defaultConfig.outputChannels.setRange(0, 2, true);
        
        return defaultConfig;
    }

private:
    juce::File configFile;
};

}

