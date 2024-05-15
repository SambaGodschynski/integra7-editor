#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include <components/PartInfo.h>
#include "SnaInstrumentSelector.h"
#include <components/FlexContainer.h>
#include <integra7/Integra7Defs.h>
#include <memory>

namespace ted_sna
{
    class Instrument : public FlexContainer
    {
    public:
        Instrument(I7Host*, const PartInfo&);
    private:
        std::shared_ptr<SnaInstrumentSelector> instrumentSelector;
        void onInstrumentChanged(const i7::SnaInstr&);
        void updateModControls(const i7::SnaInstr&);
        int indexStartModParameter = -1;
    };
}