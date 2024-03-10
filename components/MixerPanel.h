#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "MixerChannelStrip.h"

class MixerPanel : public juce::Component
{
public:
    MixerPanel();
private:
    enum 
    {
        NumChannelStrips = 16
    };
    MixerChannelStrip channelStrips[NumChannelStrips];
    juce::FlexBox flexBox;
};