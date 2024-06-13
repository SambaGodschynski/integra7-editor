#include "SnaInstrumentSelector.h"
#include <integra7/Model.h>
#include <stdexcept>
#include <integra7/SnaInstr.h>
#include <sstream>

namespace
{
    std::string instrumentToString(const i7::SnaInstr& instr)
    {
        std::stringstream ss;
        ss << "[" << instr.bank << "] " << instr.desc;
        return ss.str();
    }
}

SnaInstrumentSelector::SnaInstrumentSelector() : SearchableCombobox()
{
    setDataSource(
        []() { 
            return i7::NumSnAcousticInstruments; 
        },
        [](size_t index) { return instrumentToString(i7::SnaInstruments[index]); },
        [](size_t index, const juce::String &str)
        {
            auto queries = juce::StringArray::fromTokens(str, true);
            bool match = true;
            for (int i = 0; i < queries.size(); ++i)
            {
                const auto& query = queries.getReference(i);
                juce::String name(instrumentToString(i7::SnaInstruments[index]));
                match &= name.containsIgnoreCase(query);
            }
            return match;
        }
    );
    i7currentInstrument = &i7::SnaInstruments[0];
    selectionChanged = std::bind(&SnaInstrumentSelector::onSelectionChanged, this, std::placeholders::_1);
}

void SnaInstrumentSelector::i7setControlLimits(i7::UInt min, i7::UInt max)
{

}
void SnaInstrumentSelector::i7setValue(ControlerValueType v)
{
    Base::setSelectionIndex(v);
    if (v > i7::NumSnAcousticInstruments)
    {
        throw std::runtime_error("NumSnAcousticInstruments out of bounds");
    }
    i7currentInstrument = &i7::SnaInstruments[v];
}

void SnaInstrumentSelector::onSelectionChanged(int index)
{
    if (index < 0 || index > i7::NumSnAcousticInstruments)
    {
        throw std::runtime_error("NumSnAcousticInstruments out of bounds");
    }
    i7onValueChanged(index);
    i7currentInstrument = &i7::SnaInstruments[index];
    i7putValue(i7PartInfo.createId("_SNTONE-_SNTC-SNTC_INST_BS_LSB").c_str(), i7currentInstrument->lsb);
    i7putValue(i7PartInfo.createId("_SNTONE-_SNTC-SNTC_INST_BS_PC").c_str(), i7currentInstrument->pc);
    if (i7InstrumentChanged)
    {
        i7InstrumentChanged(*i7currentInstrument);
    }
}

void SnaInstrumentSelector::i7ModelValueChanged(ControlerValueType)
{
    auto lsb = i7getValue(i7PartInfo.createId("_SNTONE-_SNTC-SNTC_INST_BS_LSB").c_str());
    auto pc = i7getValue(i7PartInfo.createId("_SNTONE-_SNTC-SNTC_INST_BS_PC").c_str());
    for (size_t i = 0; i < i7::NumSnAcousticInstruments; ++i)
    {
        const auto& instrument = i7::SnaInstruments[i];
        if (instrument.lsb == lsb && pc == instrument.pc)
        {
            i7setValue(i);
            if (i7InstrumentChanged)
            {
                i7InstrumentChanged(*i7currentInstrument);
            }
            break;
        }
    }
}