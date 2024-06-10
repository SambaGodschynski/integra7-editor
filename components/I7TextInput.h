#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <integra7/Integra7Defs.h>

class I7TextInput : public juce::Label
{
public:
    typedef const char* ControlerValueType;
    I7TextInput();
    virtual ~I7TextInput() = default;
    void i7setControlLimits(i7::UInt min, i7::UInt max);
    void i7setValue(ControlerValueType v);
    ControlerValueType i7GetDefaultValue(i7::UInt) const { return ""; }
protected:
    virtual void i7onValueChanged(ControlerValueType v) = 0;
};