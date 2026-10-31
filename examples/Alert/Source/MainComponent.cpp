#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible (showAlertButton);
    showAlertButton.onClick = [this]
    {
        new r2juce::R2AlertComponent (this,
                                      "R2JUCE Alert",
                                      "Choose OK or Cancel.",
                                      { "OK", "Cancel" },
                                      false,
                                      [] (int) {});
    };

    setSize (600, 400);
}

MainComponent::~MainComponent()
{
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setFont (juce::Font (juce::FontOptions (16.0f)));
    g.setColour (juce::Colours::white);
    g.drawText ("Hello World!", getLocalBounds(), juce::Justification::centred, true);
}

void MainComponent::resized()
{
    showAlertButton.setBounds (getLocalBounds().withSizeKeepingCentre (160, 32));


}
