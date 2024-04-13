#include "PatchSelector.h"
#include <iostream>
#include <integra7/presets/SnAcousticPresets.h>

PatchSelector::PatchSelector() : SearchableCombobox()
{
    setDataSource(
        []() { return i7::NumSnAcousticPresets; }, 
        [](size_t index) { return juce::String(i7::SnAcousticPresets[index].fullName); },
        [](size_t index, const juce::String &str)
        {
            juce::String name(i7::SnAcousticPresets[index].fullName);
            return name.containsIgnoreCase(str);
        }
    );
}
