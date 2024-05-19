#include "Instrument.h"
#include <vector>
#include <components/I7Parameter.h>
#include <components/I7Slider.h>
#include <tuple>

namespace ted_sna
{
    	namespace
	{
		template<class TControl>
        std::tuple<std::shared_ptr<FlexContainer>, std::shared_ptr<I7Parameter<TControl>>> CreateControl(const PartInfo& partInfo, I7Host*& _i7Host, const char* id)
		{
			auto flexContainer = std::make_shared<FlexContainer>();
			auto param = std::make_shared<I7Parameter<TControl>>(partInfo.createId(id).c_str(), _i7Host);
			
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
			return std::make_tuple(flexContainer, param);
		}

		std::tuple<std::shared_ptr<FlexContainer>, std::shared_ptr<I7Parameter<I7Slider>>> CreateSlider(const PartInfo& partInfo, I7Host*& _i7Host, const char* id)
		{
			std::shared_ptr<FlexContainer> flexContainer;
			std::shared_ptr<I7Parameter<I7Slider>> control;
			std::tie(flexContainer, control) = CreateControl<I7Slider>(partInfo, _i7Host, id);
			control->setSize(150, 50);
			flexContainer->setSize(0, 50);
			control->setSliderStyle(juce::Slider::LinearHorizontal);
			control->setTextBoxStyle(juce::Slider::TextBoxRight, false, 30, 20);
			return std::make_tuple(flexContainer, control);
		}
	}

    Instrument::Instrument(I7Host* _i7Host, const PartInfo& partInfo) : 
        FlexContainer("Instrument")
    {
        float width = (float)getWidth();
		const i7::SnaInstr* initInstrument = nullptr;
        flexBox().flexDirection = juce::FlexBox::Direction::column;
        {
			std::shared_ptr<FlexContainer> flexContainer;
			std::shared_ptr<I7Parameter<SnaInstrumentSelector>> control;
			std::tie(flexContainer, control) = CreateControl<SnaInstrumentSelector>(partInfo, _i7Host, "_SNTONE-_SNTC-SNTC_INST_BS_PC");
			control->i7PartInfo = partInfo;
			control->i7InstrumentChanged = std::bind(&Instrument::onInstrumentChanged, this, std::placeholders::_1);
			initInstrument = control->i7currentInstrument;
			juce::FlexItem flexItem(width, (float)flexContainer->getHeight(), *flexContainer);
			addToFlexBox(flexContainer);
		}
		indexStartModParameter = flexBox().items.size();
		for (int i = 0; i < i7::SnaInstr::NumMods; ++i)
		{
			auto modId = std::string("_SNTONE-_SNTC-SNTC_MOD_PRM") + std::to_string(i+1);
			std::shared_ptr<FlexContainer> flexContainer;
			std::tie(flexContainer, std::ignore) = CreateSlider(partInfo, _i7Host, modId.c_str());
			juce::FlexItem flexItem(width, (float)flexContainer->getHeight(), *flexContainer);
			addToFlexBox(flexContainer);
		}
		jassert(initInstrument != nullptr);
		updateModControls(*initInstrument);
    }

	int Instrument::calculateNeededHeight() const
	{
		float total = 0.0f;
		auto n = getNumChildComponents();
		for (int i = 0; i < n; ++i)
		{
			auto component = getChildComponent(i);
			total += component->getHeight();
		}
		return (int)ceil(total);
	}

	void Instrument::onInstrumentChanged(const i7::SnaInstr& instrument)
	{
		updateModControls(instrument);
	}

	void Instrument::updateModControls(const i7::SnaInstr& instrument)
	{
		jassert(indexStartModParameter > 0);
		for (int i = 0; i < i7::SnaInstr::NumMods; ++i)
		{
			int modIndex = indexStartModParameter + i;
			auto& flexItem = flexBox().items.getReference(modIndex);
			auto modName = instrument.mods[i];
			bool isModVisible = modName != nullptr;
			if (isModVisible) 
			{
				// todo: combine isModVisible and this assert as one expression
				jassert(i < instrument.modInstrumentTable->numMods);
			}
			const i7::ModDef* modDef = &instrument.modInstrumentTable->mod[i];
			if (std::string(modDef->id) == std::string("MOD_SNSTD_VARIATION"))
			{
				isModVisible &= instrument.modInstrumentTable->vari != nullptr;
			}
			if (std::string(modDef->id) == std::string("MOD_SNSTD_PLAY_SCALE"))
			{
				isModVisible &= instrument.modInstrumentTable->scale != nullptr;
			}
			if (isModVisible)
			{
				flexItem.order = modIndex;
				flexItem.associatedComponent->setVisible(true);
				auto label = dynamic_cast<juce::Label*>(flexItem.associatedComponent->getChildComponent(0));
				jassert(label);
				label->setText(modName, juce::NotificationType::dontSendNotification);
				auto slider = dynamic_cast<I7Parameter<I7Slider>*>(flexItem.associatedComponent->getChildComponent(1));
				jassert(slider);
				if (std::string(modDef->id) == std::string("MOD_SNSTD_VARIATION"))
				{
					modDef = instrument.modInstrumentTable->vari;
				}
				if (std::string(modDef->id) == std::string("MOD_SNSTD_PLAY_SCALE"))
				{
					modDef = instrument.modInstrumentTable->scale;
				}
				slider->i7setControlLimits(modDef->min, modDef->max);
				slider->i7setValue(modDef->init);
			}
			else 
			{
				flexItem.order = 99999;
				flexItem.associatedComponent->setVisible(false);
			}
		}
		resized();
	}
}