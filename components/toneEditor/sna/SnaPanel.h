#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include <components/PartInfo.h>
#include "Common.h"
#include "Instrument.h"

namespace ted_sna
{
    class SnaPanel : public juce::Viewport
    {
    public:
        SnaPanel(I7Host*, const PartInfo &partInfo);
        virtual void resized() override;
    private:
        juce::Component viewChild;
        juce::FlexBox flexBox;
        Common commonPanel;
        Instrument instrumentPannel;
    };
}