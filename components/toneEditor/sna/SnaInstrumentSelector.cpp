#include "SnaInstrumentSelector.h"
#include <integra7/SnaInstr.h>
#include <integra7/Model.h>

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
}

void SnaInstrumentSelector::i7setControlLimits(i7::UInt min, i7::UInt max)
{

}
void SnaInstrumentSelector::i7setValue(ControlerValueType v)
{
    Base::setSelectionIndex(v);
}