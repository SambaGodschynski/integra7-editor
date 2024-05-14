#include "SnaInstrumentSelector.h"
#include <integra7/SnaInstr.h>
#include <integra7/Model.h>
#include <functional>
#include <stdexcept>

SnaInstrumentSelector::SnaInstrumentSelector() : SearchableCombobox()
{
    setDataSource(
        []() { 
            return i7::NumSnAcousticInstruments; 
        },
        [](size_t index) { return juce::String(i7::SnaInstruments[index].desc); },
        [](size_t index, const juce::String &str)
        {
            juce::String name(i7::SnaInstruments[index].desc);
            return name.containsIgnoreCase(str);
        }
    );
    selectionChanged = std::bind(&SnaInstrumentSelector::onSelectionChanged, this, std::placeholders::_1);
}

void SnaInstrumentSelector::i7setControlLimits(i7::UInt min, i7::UInt max)
{

}
void SnaInstrumentSelector::i7setValue(ControlerValueType v)
{
    Base::setSelectionIndex(v);
}

void SnaInstrumentSelector::onSelectionChanged(int index)
{
    if (index < 0 || index > i7::NumSnAcousticInstruments)
    {
        throw std::runtime_error("NumSnAcousticInstruments out of bounds");
    }
    i7onValueChanged(index);
    const auto& instrument = i7::SnaInstruments[index];
    i7putValue(i7PartInfo.createId("_SNTONE-_SNTC-SNTC_INST_BS_LSB").c_str(), instrument.lsb);
    i7putValue(i7PartInfo.createId("_SNTONE-_SNTC-SNTC_INST_BS_PC").c_str(), instrument.pc);

}