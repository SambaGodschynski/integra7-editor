#include "Instrument.h"
#include <vector>
#include <components/I7Parameter.h>


namespace ted_sna
{
    	namespace
	{
		// auto CreateSlider(const PartInfo& partInfo, I7Host*& _i7Host, const char* id)
		// {
		// 	auto flexContainer = std::make_shared<FlexContainer>();
		// 	auto param = std::make_shared<I7Parameter<I7Slider>>(partInfo.createId(id).c_str(), _i7Host);
		// 	auto label = std::make_shared<juce::Label>();
		// 	flexContainer->flexBox().flexDirection = juce::FlexBox::Direction::row;
		// 	flexContainer->flexBox().flexWrap = juce::FlexBox::Wrap::noWrap;

		// 	label->setText(param->i7getDescription(), juce::NotificationType::dontSendNotification);
		// 	label->setSize(70, 50);

		// 	param->setSize(150, 50);
		// 	param->setSliderStyle(juce::Slider::LinearHorizontal);
		// 	param->setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);

		// 	flexContainer->setSize(0, 50);
		// 	flexContainer->addToFlexBox(label);
		// 	auto flexItem = flexContainer->addToFlexBox(param);
		// 	flexItem->flexGrow = 1;
		// 	return flexContainer;
		// }

        auto TodoRefactor(const PartInfo& partInfo, I7Host*& _i7Host, const char* id)
		{
			auto flexContainer = std::make_shared<FlexContainer>();
			auto param = std::make_shared<I7Parameter<SnaInstrumentSelector>>(partInfo.createId(id).c_str(), _i7Host);

            param->setBounds(0, 0, 150, 0);

			auto label = std::make_shared<juce::Label>();
			flexContainer->flexBox().flexDirection = juce::FlexBox::Direction::row;
			flexContainer->flexBox().flexWrap = juce::FlexBox::Wrap::noWrap;

			label->setText(param->i7getDescription(), juce::NotificationType::dontSendNotification);
			label->setSize(70, 30);
            param->setSize(150, 30);
            
			flexContainer->setSize(0, 30);
			flexContainer->addToFlexBox(label);
			auto flexItem = flexContainer->addToFlexBox(param);
			flexItem->flexGrow = 1;
			return flexContainer;
		}
	}

    Instrument::Instrument(I7Host* _i7Host, const PartInfo& partInfo) : 
        FlexContainer("Instrument")
    {
        float width = getWidth();
        flexBox().flexDirection = juce::FlexBox::Direction::column;
        {
			auto param = TodoRefactor(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_INST_BS_PC");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
        resized();
    }
}