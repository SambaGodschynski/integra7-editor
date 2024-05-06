#include "PatchSelector.h"
#include <integra7/SnAcousticPresets.h>
#include <integra7/SnaInstr.h>
#include <integra7/Model.h>

namespace
{
    constexpr size_t TotalNumPresets = i7::NumSnAcousticPresets;
    const i7::Preset* Presets[TotalNumPresets];
    bool _createTotalInstruments()
    {
        size_t total = 0;
        for (size_t i = 0; i < i7::NumSnAcousticPresets; ++i)
        {
            Presets[total++] = &i7::SnAcousticPresets[i];
        }
        return true;
    }
    bool _ = _createTotalInstruments();
}

PatchSelector::PatchSelector() : SearchableCombobox()
{
    setDataSource(
        []() { return TotalNumPresets; },
        [](size_t index) { return juce::String(Presets[index]->fullName); },
        [](size_t index, const juce::String &str)
        {
            juce::String name(Presets[index]->fullName);
            return name.containsIgnoreCase(str);
        }
    );
}
