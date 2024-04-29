#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

class I7Slider : public juce::Component
{
public:
    typedef unsigned int ControlerValueType;
    I7Slider();
    void setControlLimits(int min, int max);
    void setValue(ControlerValueType v);
protected:
    virtual void onValueChanged(ControlerValueType v) = 0;
};