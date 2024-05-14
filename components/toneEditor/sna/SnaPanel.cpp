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
        int w = (int)getWidth() - 15;
        commonPanel.setSize(w, 600);
        flexBox.items.getReference(0).width = (float)commonPanel.getWidth();
        flexBox.items.getReference(0).height = (float)commonPanel.getHeight();

        instrumentPannel.setSize(w, 1000);
        flexBox.items.getReference(1).width = (float)instrumentPannel.getWidth();
        flexBox.items.getReference(1).height = (float)instrumentPannel.getHeight();

        int totalHeight = (int)(commonPanel.getHeight() + instrumentPannel.getHeight());

        viewChild.setSize(w, totalHeight);
        auto bounds = juce::Rectangle<int>(0, 0, viewChild.getWidth(), viewChild.getHeight());
        flexBox.performLayout(bounds);
    }
}