#include "SnaPanel.h"
#include <vector>

namespace ted_sna
{
  SnaPanel::SnaPanel(I7Host* _i7Host, const PartInfo& partInfo) :
        juce::Viewport("SnaPanel"),
        commonPanel(_i7Host, partInfo),
        instrumentPannel(_i7Host, partInfo)
    {
        flexBox.flexDirection = juce::FlexBox::Direction::column;
        viewChild.addAndMakeVisible(commonPanel);
        flexBox.items.add(juce::FlexItem(0, 0, commonPanel));

        viewChild.addAndMakeVisible(instrumentPannel);
        flexBox.items.add(juce::FlexItem(0, 0, instrumentPannel));

        setViewedComponent(&viewChild, false);
        resized();
    }

    void SnaPanel::resized()
    {
        int w = getWidth() - 15;
        commonPanel.setSize(w, 600);
        flexBox.items.getReference(0).width = commonPanel.getWidth();
        flexBox.items.getReference(0).height = commonPanel.getHeight();

        instrumentPannel.setSize(w, 1000);
        flexBox.items.getReference(1).width = instrumentPannel.getWidth();
        flexBox.items.getReference(1).height = instrumentPannel.getHeight();

        int totalHeight = commonPanel.getHeight() + instrumentPannel.getHeight();

        viewChild.setSize(w, totalHeight);
        auto bounds = juce::Rectangle<int>(0, 0, viewChild.getWidth(), viewChild.getHeight());
        flexBox.performLayout(bounds);
    }
}