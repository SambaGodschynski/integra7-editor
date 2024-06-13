#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <integra7/Integra7Defs.h>
#include <string>

class I7Slider : public juce::Slider
{
public:
    typedef unsigned int ControlerValueType;
    I7Slider();
    virtual ~I7Slider() = default;
    void i7setControlLimits(i7::UInt min, i7::UInt max);
    void i7setValue(ControlerValueType v);
    ControlerValueType i7GetDefaultValue(i7::UInt initValue) const { return initValue; }
    virtual void valueChanged() override;
protected:
    virtual void i7onValueChanged(ControlerValueType v) = 0;
    virtual i7::UInt i7getValue(const char* nodeId) const = 0;
    virtual std::string i7getStrValue(const char* nodeId) const = 0;
    void i7ModelValueChanged(ControlerValueType v) { i7setValue(v); }
};