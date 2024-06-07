#include "ToneEditorPanel.h"
#include <vector>

namespace ted
{
    ToneEditorPanel::ToneEditorPanel(I7Host* _i7Host) :
        FlexContainer("tone editor panel")
    {
        header = std::make_shared<ted::Header>(_i7Host, partInfo);
        snaPannel = std::make_shared<ted_sna::SnaPanel>(_i7Host, partInfo);
        flexBox().flexDirection = juce::FlexBox::Direction::column;
        header->setSize(0, 50);
        addToFlexBox(header);
        auto item = addToFlexBox(snaPannel);
        item->flexGrow = 1;
    }
}
