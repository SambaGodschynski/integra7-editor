#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

class MixerChannelStrip : public juce::Component
{
public:
    MixerChannelStrip();
    virtual void resized() override;
private:
    juce::Slider volume;
    juce::Slider pan;
    juce::FlexBox flexBox;
};