#ifdef R2JUCE_H_INCLUDED
  /* This file is designed to be compiled as a single unit, so you should not include
     it in any other file.
  */
  #error "Incorrect use of R2JUCE.cpp"
#endif

#include "R2JUCE.h"

#include "src/R2LookAndFeel.cpp"
#include "src/R2TextEditor.cpp"
#include "src/R2Label.cpp"
#include "src/R2Slider.cpp"
#include "src/R2Led.cpp"
#include "src/R2WiFiSelector.cpp"
#include "src/R2ScreenKeyboard.cpp"
#include "src/R2LocalStorageProvider.cpp"
#include "src/R2GoogleDriveProvider.cpp"
#include "src/R2OneDriveProvider.cpp"
#include "src/R2CloudManager.cpp"
#include "src/R2CloudAuthComponent.cpp"
#include "src/R2AlertComponent.cpp"
