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

//[Headers] You can add your own extra header files here...
//[/Headers]

#include "CloudDocAudioProcessorEditor.h"


//[MiscUserDefs] You can add your own user definitions and misc code here...
//[/MiscUserDefs]

//==============================================================================
CloudDocAudioProcessorEditor::CloudDocAudioProcessorEditor (CloudDocAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //[Constructor_pre] You can add your own custom stuff here..
    //[/Constructor_pre]


    //[UserPreSize]
    mainComponent.reset(new MainComponent);
    addAndMakeVisible(mainComponent.get());
    //[/UserPreSize]

    setSize (800, 480);


    //[Constructor] You can add your own custom stuff here..
#if JUCE_IOS || JUCE_ANDROID || JUCE_RASPI
    //  For mobile devices with fullscreen, set editor size as desktop size.
    auto r = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    setSize (r.getWidth(), r.getHeight());
#elif JUCE_RASPI_SIMULATE
    //  When simulating Raspberry Pi, use defined size in Projucer.
    setSize (JUCE_RASPI_SIMULATE_WIDTH, JUCE_RASPI_SIMULATE_HEIGHT);
#endif
    //[/Constructor]
}

CloudDocAudioProcessorEditor::~CloudDocAudioProcessorEditor()
{
    //[Destructor_pre]. You can add your own custom destruction code here..
    mainComponent = nullptr;
    //[/Destructor_pre]



    //[Destructor]. You can add your own custom destruction code here..
    //[/Destructor]
}

//==============================================================================
void CloudDocAudioProcessorEditor::paint (juce::Graphics& g)
{
    //[UserPrePaint] Add your own custom painting code here..
    //[/UserPrePaint]

    g.fillAll (juce::Colours::black);

    //[UserPaint] Add your own custom painting code here..
    //[/UserPaint]
}

void CloudDocAudioProcessorEditor::resized()
{
    //[UserPreResize] Add your own custom resize code here..
    //[/UserPreResize]

    //[UserResized] Add your own custom resize handling here..
    if (mainComponent != nullptr)
    {
        // エディターのサイズを取得
        int editorWidth = getWidth();
        int editorHeight = getHeight();

        // mainComponentの元のサイズ（基準サイズ）
        int componentOriginalWidth = 568;  // 元の幅
        int componentOriginalHeight = 320; // 元の高さ

        // アスペクト比を計算
        float componentAspectRatio = (float)componentOriginalWidth / (float)componentOriginalHeight;
        float editorAspectRatio = (float)editorWidth / (float)editorHeight;

        float scaleX, scaleY, offsetX, offsetY;

        if (componentAspectRatio > editorAspectRatio)
        {
            // mainComponentの方が横長 → 幅に合わせてスケール
            scaleX = scaleY = (float)editorWidth / (float)componentOriginalWidth;
            offsetX = 0;
            offsetY = (editorHeight - componentOriginalHeight * scaleY) * 0.5f; // 縦方向センタリング
        }
        else
        {
            // mainComponentの方が縦長（または同じ比率） → 高さに合わせてスケール
            scaleX = scaleY = (float)editorHeight / (float)componentOriginalHeight;
            offsetX = (editorWidth - componentOriginalWidth * scaleX) * 0.5f; // 横方向センタリング
            offsetY = 0;
        }

        // mainComponentを元のサイズに設定
        mainComponent->setBounds(0, 0, componentOriginalWidth, componentOriginalHeight);

        // AffineTransformを適用してスケール&オフセット
        juce::AffineTransform transform = juce::AffineTransform::scale(scaleX, scaleY)
                                                                .translated(offsetX, offsetY);
        mainComponent->setTransform(transform);
    }
    //[/UserResized]
}



//[MiscUserCode] You can add your own definitions of your custom methods or any other code here...
//[/MiscUserCode]


//==============================================================================
#if 0
/*  -- Projucer information section --

    This is where the Projucer stores the metadata that describe this GUI layout, so
    make changes in here at your peril!

BEGIN_JUCER_METADATA

<JUCER_COMPONENT documentType="Component" className="CloudDocAudioProcessorEditor"
                 componentName="" parentClasses="public juce::AudioProcessorEditor"
                 constructorParams="CloudDocAudioProcessor&amp; p" variableInitialisers="AudioProcessorEditor (&amp;p), audioProcessor (p)"
                 snapPixels="8" snapActive="1" snapShown="1" overlayOpacity="0.330"
                 fixedSize="0" initialWidth="800" initialHeight="480">
  <BACKGROUND backgroundColour="ff000000"/>
</JUCER_COMPONENT>

END_JUCER_METADATA
*/
#endif


//[EndFile] You can add extra defines here...
//[/EndFile]

