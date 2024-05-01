#include "Common.h"
#include <vector>

namespace ted_sna
{
    Common::Common(I7Host* _i7Host, const PartInfo& partInfo) : 
        juce::Viewport("Tone Editor Sna Common")
    {
        auto createId = [&partInfo](const char* id) { return std::string("PRM-_FPART") + std::to_string(partInfo.partId) + id + "-"; };
        
        //{
        //    juce::FlexItem flexItem((float)volume.getWidth(), (float)volume.getHeight(), volume);
        //    flexBox.items.add(flexItem);
        //    addAndMakeVisible(volume);
        //}
    }
}