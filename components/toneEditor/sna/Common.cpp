#include "Common.h"
#include <vector>
#include <components/I7Slider.h>
#include <components/FlexContainer.h>

namespace ted_sna
{

	namespace
	{
		auto CreateSlider(const PartInfo& partInfo, I7Host*& _i7Host, const char* id)
		{
			std::shared_ptr<FlexContainer> flexContainer = std::make_shared<FlexContainer>();
			std::shared_ptr<juce::Slider> param = std::make_shared<I7Parameter<I7Slider>>(partInfo.createId(id).c_str(), _i7Host);
			flexContainer->flexBox().flexDirection = juce::FlexBox::Direction::column;
			param->setSize(0, 50);
			param->setSliderStyle(juce::Slider::LinearHorizontal);
			param->setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);
			flexContainer->setSize(1000, 50);
			flexContainer->addToFlexBox(param);
			return flexContainer;
		}
	}

	Common::Common(I7Host* _i7Host, const PartInfo& partInfo) :
		juce::Component("Tone Editor Sna Common")
	{
		float width = getWidth();
		flexBox.flexDirection = juce::FlexBox::Direction::column;
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_PHRASE_OCT");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTONE-_SNTC-SNTC_TONE_LEVEL");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		// TODO: MONO POLY
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_OCTAVE");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_CUTOFF_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_RESO_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_ATK_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_REL_OFST");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_PORT_TIME");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_VIB_RATE");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_VIB_DEPTH");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		{
			auto param = CreateSlider(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_VIB_DELAY");
			juce::FlexItem flexItem(width, (float)param->getHeight(), *param);
			flexBox.items.add(flexItem);
			addAndMakeVisible(*param);
			parameters.push_back(param);
		}
		resized();
	}

	void Common::resized()
	{
		const int w = getWidth();
		for (size_t i = 0; i < parameters.size(); ++i)
		{
			auto param = parameters.at(i);
			param->setSize(w, param->getHeight());
			flexBox.items.getReference((int)i).width = (float)w;
		}
		auto bounds = juce::Rectangle<int>(0, 0, getWidth(), getHeight());
		flexBox.performLayout(bounds);
	}
}