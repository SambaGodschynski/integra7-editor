#include "MixerChannelStrip.h"

MixerChannelStrip::MixerChannelStrip() : juce::Component("mixer channel strip")
{
    setSize(50, 120);
    slider.setSize(50, 120);
    slider.setSliderStyle( juce::Slider::LinearVertical );
    slider.setTextBoxStyle( juce::Slider::TextBoxAbove, false, 60, 20);
    slider.setRange(0, 127, 1);
    addAndMakeVisible(slider);
}