#pragma once

#define R2JUCE_H_INCLUDED

/** BEGIN_JUCE_MODULE_DECLARATION
    ID:             R2JUCE
    vendor:         ring2
    version:        0.0.1
    name:           R2 JUCE Module
    description:    A collection of custom JUCE components
    license:        MIT
    dependencies:   juce_gui_basics, juce_audio_devices
    website:        https://studio-r.tokyo/
    minimumCppStandard: 17
END_JUCE_MODULE_DECLARATION
*/

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>

// Include all public headers of your module here.
#include "src/R2CloudStorageProvider.h"
#include "src/R2LocalStorageProvider.h"
#include "src/R2GoogleDriveProvider.h"
#include "src/R2OneDriveProvider.h"
#include "src/R2CloudManager.h"
#include "src/R2CloudAuthComponent.h"
#include "src/R2AlertComponent.h"
#include "src/R2LookAndFeel.h"
#include "src/R2TextEditor.h"
#include "src/R2Label.h"
#include "src/R2Slider.h"
#include "src/R2Led.h"
#include "src/R2WiFiSelector.h"
#include "src/R2ScreenKeyboard.h"
#include "src/R2AudioConfigManager.h"

#if JUCE_MAC || JUCE_IOS
#include "src/R2IcloudDriveProvider.h"
#endif
