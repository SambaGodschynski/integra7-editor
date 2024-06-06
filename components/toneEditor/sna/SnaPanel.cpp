#include "SnaPanel.h"
#include <vector>
#include <components/Common.h>

namespace ted_sna
{
  SnaPanel::SnaPanel(I7Host* _i7Host, const PartInfo& partInfo) :
        juce::Viewport("SnaPanel"),
        commonPanel(_i7Host, partInfo),
        instrumentPannel(_i7Host, partInfo)
    {

        flexBox.flexDirection = juce::FlexBox::Direction::column;
        receiveBtn.setSize(100, 50);
        receiveBtn.setButtonText("Receive");
        receiveBtn.onClick = [_i7Host]()
        {
            try 
            {
                _i7Host->requestExpansion();
            }
            catch (const std::exception &ex)
            {
                I7Alert(ex.what());
            }
        };
        viewChild.addAndMakeVisible(receiveBtn);
        flexBox.items.add(juce::FlexItem(0, 0, receiveBtn));

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

        auto iPanelHeight = instrumentPannel.calculateNeededHeight();
        instrumentPannel.setSize(w, iPanelHeight);
        flexBox.items.getReference(1).width = (float)instrumentPannel.getWidth();
        flexBox.items.getReference(1).height = (float)instrumentPannel.getHeight();

        int totalHeight = (int)(commonPanel.getHeight() + instrumentPannel.getHeight());

        viewChild.setSize(w, totalHeight);
        auto bounds = juce::Rectangle<int>(0, 0, viewChild.getWidth(), viewChild.getHeight());
        flexBox.performLayout(bounds);
    }
}