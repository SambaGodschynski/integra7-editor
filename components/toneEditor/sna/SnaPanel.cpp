#include "SnaPanel.h"
#include <vector>

namespace ted_sna
{
    SnaPanel::SnaPanel(I7Host* _i7Host, const PartInfo& partInfo) :
        juce::Viewport("SnaPanel"),
        commonPanel(_i7Host, partInfo)
    {
        addAndMakeVisible(commonPanel);

        resized();
    }

    void SnaPanel::resized()
    {

    }
}