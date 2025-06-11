#pragma once

#include <JuceHeader.h>

namespace r2juce {

class R2Led    : public juce::Component
{
public:
    R2Led(juce::Image theImage, int numSubImages);
    ~R2Led();
    
    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent &event) override;
    void mouseUp (const juce::MouseEvent &event) override;
    
    void setImage(const juce::Image image, int numSubImages);
    void setValue (float newValue);
    float getValue () const { return value; }
    int getNumSubImages() { return numSubImages; }
    
    void onMouseDown(std::function<void()> callback) {
        mouseDownCallback = callback;
    }
    
    void onMouseUp(std::function<void()> callback) {
        mouseUpCallback = callback;
    }
    
private:
    juce::Image image;
    int numSubImages;
    float value;
    
    std::function<void()> mouseDownCallback = nullptr;
    std::function<void()> mouseUpCallback = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (R2Led)
};
}   //  namespace
