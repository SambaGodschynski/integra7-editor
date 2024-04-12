#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "SearchableCombobox.h"

class ToneEditorPanel : public juce::Component
{
public:
    ToneEditorPanel();
private:
    SearchableCombobox patchSelector;
    juce::Label testLabel;
};