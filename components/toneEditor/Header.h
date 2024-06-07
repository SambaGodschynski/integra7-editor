#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include <components/FlexContainer.h>
#include <components/PartInfo.h>
#include <memory>

namespace ted 
{
    class Header : public FlexContainer
    {
    public:
        Header(I7Host*, const PartInfo&);
    private:
        std::shared_ptr<juce::TextButton> receiveBtn;
    };
}

