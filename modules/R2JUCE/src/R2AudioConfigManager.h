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
        // 設定ファイル用のディレクトリを作成
        configFile.getParentDirectory().createDirectory();
    }

    virtual ~R2AudioConfigManager() = default;

    // 設定をファイルから読み込み（ファイルがなければ初期値を使用）
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
                    
                    // チャンネル設定の読み込み
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
                DBG("設定ファイルの読み込みに失敗しました。初期値を使用します。");
            }
        }
        else
        {
            DBG("設定ファイルが見つかりません。初期値を使用します。");
        }
        
        // ファイルがない場合や読み込みに失敗した場合は初期値を返す
        return getDefaultConfig();
    }

    // 現在の設定をファイルに保存
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
            
            // タイムスタンプも保存
            jsonObject->setProperty("lastSaved", juce::Time::getCurrentTime().toString(true, true));
            
            juce::String jsonText = juce::JSON::toString(juce::var(jsonObject.get()), true);
            
            return configFile.replaceWithText(jsonText);
        }
        catch (...)
        {
            DBG("設定ファイルの保存に失敗しました");
            return false;
        }
    }

    // AudioDeviceManagerから現在の設定を取得してAudioConfigに変換
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

    // AudioConfigをAudioDeviceSetupに変換
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

    // 設定ファイルのパスを取得
    juce::File getConfigFile() const { return configFile; }

    // 設定ファイルが存在するかチェック
    bool configFileExists() const { return configFile.existsAsFile(); }

protected:
    // 初期値を取得（継承先でオーバーライド可能）
    virtual AudioConfig getDefaultConfig()
    {
        AudioConfig defaultConfig;
        
        // デフォルトの初期値
        defaultConfig.outputDeviceName = "";  // 空文字列の場合、システムデフォルトを使用
        defaultConfig.inputDeviceName = "";
        defaultConfig.sampleRate = 44100.0;
        defaultConfig.bufferSize = 512;
        
        // デフォルトでステレオ入出力を有効化
        defaultConfig.inputChannels.setRange(0, 2, true);
        defaultConfig.outputChannels.setRange(0, 2, true);
        
        return defaultConfig;
    }

private:
    juce::File configFile;
};

}

