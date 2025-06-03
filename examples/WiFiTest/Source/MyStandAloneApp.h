#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
//#include "TwiAudioProcessorNew.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

// You can set this flag in your build if you need to specify a different
// standalone JUCEApplication class for your app to use. If you don't
// set it then by default we'll just create a simple one as below.
//#if ! JUCE_USE_CUSTOM_PLUGIN_STANDALONE_APP

extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

//  On iOS and Android, force disable inputs to avoid consuming mic input due to rare use case.
#if JUCE_IOS || JUCE_ANDROID
#undef JucePlugin_PreferredChannelConfigurations
#define JucePlugin_PreferredChannelConfigurations  {0, 2}
#endif

//==============================================================================
class MyStandAloneApp : public juce::JUCEApplication
{
public:
    MyStandAloneApp();
    ~MyStandAloneApp() {}

    const juce::String getApplicationName() override              { return JucePlugin_Name; }
    const juce::String getApplicationVersion() override           { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override              { return false; }
    void anotherInstanceStarted (const juce::String& pathToFile) override;

    virtual juce::StandaloneFilterWindow* createWindow();

    //==============================================================================
    void initialise (const juce::String&) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void suspended() override;
    void resumed() override;

protected:
    juce::ApplicationProperties appProperties;
    std::unique_ptr<juce::StandaloneFilterWindow> mainWindow;
};
