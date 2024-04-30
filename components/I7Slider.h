#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <integra7/Integra7Defs.h>

class I7Slider : public juce::Slider
{
public:
    typedef unsigned int ControlerValueType;
    I7Slider();
    void i7setControlLimits(i7::UInt min, i7::UInt max);
    void i7setValue(ControlerValueType v);
    ControlerValueType i7GetDefaultValue(i7::UInt initValue) const { return initValue; }
    virtual void valueChanged() override;
protected:
    virtual void i7onValueChanged(ControlerValueType v) = 0;
};