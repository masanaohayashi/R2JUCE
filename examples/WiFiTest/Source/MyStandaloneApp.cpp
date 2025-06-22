#include "MyStandAloneApp.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_utils/juce_audio_utils.h>

extern juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter();

#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

//==============================================================================
MyStandAloneApp::MyStandAloneApp()
{
    juce::PluginHostType::jucePlugInClientCurrentWrapperType = juce::AudioProcessor::wrapperType_Standalone;
}

void MyStandAloneApp::anotherInstanceStarted (const juce::String& pathToFile)
{
}

juce::StandaloneFilterWindow* MyStandAloneApp::createWindow()
{
    #ifdef JucePlugin_PreferredChannelConfigurations
    juce::StandalonePluginHolder::PluginInOuts channels[] = { JucePlugin_PreferredChannelConfigurations };
    #endif

    auto* window = new juce::StandaloneFilterWindow (getApplicationName(),
                                        juce::LookAndFeel::getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                                        nullptr,
                                        false, {}, nullptr
                                        #ifdef JucePlugin_PreferredChannelConfigurations
                                        , juce::Array<juce::StandalonePluginHolder::PluginInOuts> (channels, juce::numElementsInArray (channels))
                                        #else
                                        , {}
                                        #endif
                                        #if JUCE_DONT_AUTO_OPEN_MIDI_DEVICES_ON_MOBILE
                                        , false
                                        #else //JUCE_RASPI
                                        , true
                                        #endif
                                       );
    // ここでリサイズ可能に設定
    window->setResizable(true, false);
    
    // 最小・最大サイズを設定（オプション）
    //window->setResizeLimits(400, 300, 1600, 1200);
    
    return window;

}

//==============================================================================
void MyStandAloneApp::initialise (const juce::String&)
{
    //  In this method, an instance of AudioProcessor is created, initialized, and setStateInformation() is called.
    mainWindow.reset (createWindow());
    mainWindow->setUsingNativeTitleBar(true);
    mainWindow->centreWithSize(mainWindow->getWidth(), mainWindow->getHeight());

    mainWindow->setVisible (true);

    juce::StandalonePluginHolder* holder = juce::StandalonePluginHolder::getInstance();
    jassert(holder != nullptr);
}

void MyStandAloneApp::shutdown()
{
    mainWindow = nullptr;
    //appProperties.saveIfNeeded();
}

//==============================================================================
void MyStandAloneApp::systemRequestedQuit()
{
    if (juce::ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        juce::Timer::callAfterDelay (100, []()
        {
            if (auto app = JUCEApplicationBase::getInstance())
                app->systemRequestedQuit();
        });
    }
    else
    {
        quit();
    }
}

void MyStandAloneApp::suspended()
{
}

void MyStandAloneApp::resumed()
{
}

/*
void MyStandAloneApp::saveProperties ()
{
}
*/

void TwiStandAloneAppSaveProperties()
{
/*
    auto holder = juce::StandalonePluginHolder::getInstance();
    if (holder != nullptr)
    {
        auto app = dynamic_cast<juce::TwiStandAloneApp*>(juce::JUCEApplication::getInstance());
        if (app != nullptr)
        {
            app->saveProperties();
        }
    }
*/
}

#if JucePlugin_Build_Standalone && JUCE_IOS

using namespace juce;
/*
bool JUCE_CALLTYPE juce_isInterAppAudioConnected()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->isInterAppAudioConnected();

    return false;
}

void JUCE_CALLTYPE juce_switchToHostApplication()
{
    if (auto holder = StandalonePluginHolder::getInstance())
        holder->switchToHostApplication();
}

#if JUCE_MODULE_AVAILABLE_juce_gui_basics
Image JUCE_CALLTYPE juce_getIAAHostIcon (int size)
{
    if (auto holder = StandalonePluginHolder::getInstance())
        return holder->getIAAHostIcon (size);

    return Image();
}
#endif
*/
#endif

START_JUCE_APPLICATION(MyStandAloneApp)

//#endif
