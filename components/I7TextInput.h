#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

class I7TextInput : public juce::Component
{
public:
    typedef const char* ControlerValueType;
    I7TextInput();
    void setControlLimits(int min, int max);
    void setValue(ControlerValueType v);
protected:
    virtual void onValueChanged(ControlerValueType v) = 0;
};