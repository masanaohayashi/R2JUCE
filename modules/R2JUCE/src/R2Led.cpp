#include "R2Led.h"

namespace r2juce {

//==============================================================================
R2Led::R2Led(juce::Image theImage, int numSubImages) :
image (theImage),
numSubImages (numSubImages),
value (0.0f)
{
    // In your constructor, you should add any child components, and
    // initialise any special settings that your component needs.
}

R2Led::~R2Led()
{
}

void R2Led::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
    //juce::Image image = juce::ImageCache::getFromMemory(imageName, juce::BinaryData::KnobRed2x_pngSize);
    int subImageWidth  = image.getWidth();
    int subImageHeight = image.getHeight() / numSubImages;
    
    int index = juce::jmin(juce::jmax<int>(value * numSubImages, 0), numSubImages - 1);
    
    const int destWidth = getWidth();
    const int destHeight = getHeight();
    
    const int srcX = 0;
    const int srcY = index * subImageHeight;
    g.fillAll(juce::Colour((juce::uint8) 0, (juce::uint8) 0, (juce::uint8) 0, (juce::uint8) 0));
    /*
     double scaleX = (double)destWidth  / (double)subImageWidth;
     double scaleY = (double)destHeight / (double)subImageHeight;
     AffineTransform affineTransform = AffineTransform::scale(scaleX, scaleY);
     */
    g.drawImage(image, 0, 0, destWidth, destHeight, srcX, srcY, subImageWidth, subImageHeight);
}

void R2Led::resized()
{
    // This method is where you should set the bounds of any child
    // components that your component contains..
    
}

void R2Led::mouseDown (const juce::MouseEvent &event)
{
    if (mouseDownCallback)
    {
        mouseDownCallback();
    }
}

void R2Led::mouseUp (const juce::MouseEvent &event)
{
    if (mouseUpCallback)
    {
        mouseUpCallback();
    }
}

void R2Led::setImage(const juce::Image _image, int _numSubImages)
{
    image = _image;
    numSubImages = _numSubImages;
}

void R2Led::setValue (float newValue)
{
    if (value != newValue)
    {
        value = newValue;
        repaint();
    }
}

}   //  namespaceo
