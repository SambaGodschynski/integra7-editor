#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

class SearchableCombobox : public juce::ComboBox
{
public:
    SearchableCombobox();
    bool keyStateChanged(bool) override;
};