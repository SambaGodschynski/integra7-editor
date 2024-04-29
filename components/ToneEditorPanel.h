#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "PatchSelector.h"
#include "I7Parameter.h"
#include "I7Slider.h"
#include "I7TextInput.h"

class ToneEditorPanel : public juce::Component
{
public:
    ToneEditorPanel();
private:
    PatchSelector patchSelector;
    juce::Label testLabel;
    I7Parameter<I7Slider> p1;
    I7Parameter<I7TextInput> p2;
};