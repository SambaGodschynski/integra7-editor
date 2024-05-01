#include "Instrument.h"
#include <vector>

namespace ted_sna
{
    Instrument::Instrument(I7Host* _i7Host, const PartInfo& partInfo) : 
        juce::Component("Instrument")
    {
    }
}