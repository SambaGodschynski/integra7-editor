#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/SearchableCombobox.h>
#include <integra7/Integra7Defs.h>
#include <components/I7Host.h>
#include <components/PartInfo.h>
#include <functional>
#include <string>

class SnaInstrumentSelector : public SearchableCombobox
{
public:
    typedef SearchableCombobox Base;
    SnaInstrumentSelector();
    typedef unsigned int ItemIndex;
    typedef ItemIndex ControlerValueType;
    typedef std::function<void(const i7::SnaInstr&)> InstrumentChanged;
    void i7setControlLimits(i7::UInt min, i7::UInt max);
    void i7setValue(ControlerValueType v);
    ControlerValueType i7GetDefaultValue(i7::UInt initValue) const { return initValue; }
    PartInfo i7PartInfo;
    InstrumentChanged i7InstrumentChanged;
    const i7::SnaInstr* i7currentInstrument = nullptr;
protected:
    void onSelectionChanged(int index);
    virtual void i7onValueChanged(ControlerValueType v) = 0;
    virtual void i7putValue(const char* nodeId, i7::UInt v) = 0;
    virtual i7::UInt i7getValue(const char* nodeId) const = 0;
    virtual std::string i7getStrValue(const char* nodeId) const = 0;
    void i7ModelValueChanged(ControlerValueType);
};