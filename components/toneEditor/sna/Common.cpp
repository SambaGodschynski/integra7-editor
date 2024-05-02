#include "Common.h"
#include <vector>
#include <components/I7Slider.h>

namespace ted_sna
{

	namespace
	{
		auto CreateSlider(const PartInfo& partInfo, I7Host*& _i7Host, const char* id)
		{
			auto flexContainer = std::make_shared<FlexContainer>();
			auto param = std::make_shared<I7Parameter<I7Slider>>(partInfo.createId(id).c_str(), _i7Host);
			auto label = std::make_shared<juce::Label>();
			flexContainer->flexBox().flexDirection = juce::FlexBox::Direction::row;
			flexContainer->flexBox().flexWrap = juce::FlexBox::Wrap::noWrap;

			label->setText(param->i7getDescription(), juce::NotificationType::dontSendNotification);
			label->setSize(70, 50);

			param->setSize(150, 50);
			param->setSliderStyle(juce::Slider::LinearHorizontal);
			param->setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);

			flexContainer->setSize(0, 50);
			flexContainer->addToFlexBox(label);
			auto flexItem = flexContainer->addToFlexBox(param);
			flexItem->flexGrow = 1;
			return flexContainer;
		}
	}

	Common::Common(I7Host* _i7Host, const PartInfo& partInfo) : FlexContainer("Tone Editor Sna Common")
	{
		float width = getWidth();
		flexBox().flexDirection = juce::FlexBox::Direction::column;
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_PHRASE_OCT");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTONE-_SNTC-SNTC_TONE_LEVEL");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		// TODO: MONO POLY
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_OCTAVE");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_CUTOFF_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_RESO_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_ATK_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_REL_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_PORT_TIME");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_VIB_RATE");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_VIB_DEPTH");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_VIB_DELAY");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			addToFlexBox(param);
		}
		resized();
	}
}