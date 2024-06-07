#include "Header.h"
#include <vector>
#include <components/I7Parameter.h>
#include <components/I7Slider.h>
#include <tuple>

namespace ted
{
	Header::Header(I7Host* _i7Host, const PartInfo& partInfo)
	{
		flexBox().flexDirection = juce::FlexBox::Direction::row;
		receiveBtn = std::make_shared<juce::TextButton>();
		receiveBtn->setSize(100, 0);
        receiveBtn->setButtonText("Receive");
		addToFlexBox(receiveBtn);
        receiveBtn->onClick = [_i7Host]()
        {
            try 
            {
                _i7Host->requestPart(0);
            }
            catch (const std::exception &ex)
            {
                I7Alert(ex.what());
            }
        };
	}
}