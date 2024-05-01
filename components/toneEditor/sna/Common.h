#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include <components/I7Parameter.h>
#include <components/PartInfo.h>
namespace ted_sna
{
    class Common : public juce::Viewport
    {
    public:
        Common(I7Host*, const PartInfo&);
    private:
        juce::FlexBox flexBox;
        typedef std::vector<I7ParameterBase> Parameter;
        Parameter parameter;
    };
}