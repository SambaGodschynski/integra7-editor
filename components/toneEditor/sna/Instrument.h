#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>

namespace ted_sna
{
    class Instrument : public juce::Viewport
    {
    public:
        Instrument(I7Host*);
    private:
    };
}