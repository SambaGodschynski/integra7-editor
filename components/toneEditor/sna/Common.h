#pragma once

#include <vector>
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include <components/I7Parameter.h>
#include <components/PartInfo.h>
#include <memory>
namespace ted_sna
{
    class Common : public juce::Component
    {
    public:
        Common(I7Host*, const PartInfo&);
        virtual void resized() override;
    private:
        juce::FlexBox flexBox;
        typedef std::vector<std::shared_ptr<juce::Component>> Parameters;
        Parameters parameters;
    };
}