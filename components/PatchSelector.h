
#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>

class PatchSelector : public juce::Component, public juce::ListBoxModel
{
public:
    PatchSelector();
    virtual int getNumRows() override;
    virtual void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    virtual juce::String getNameForRow (int rowNumber) override;
    virtual void resized() override;
private:
    juce::ListBox list;
};