#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include "sna/SnaPanel.h"
#include <components/PartInfo.h>
#include <components/FlexContainer.h>
#include "Header.h"

namespace ted
{
    class ToneEditorPanel : public FlexContainer
    {
    public:
        ToneEditorPanel(I7Host*);
    private:
        PartInfo partInfo;
        std::shared_ptr<ted::Header> header;
        std::shared_ptr<ted_sna::SnaPanel> snaPannel;
    };
}
