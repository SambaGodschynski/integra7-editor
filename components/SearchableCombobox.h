#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Input.h"

class SearchableCombobox : public juce::Component, public juce::ListBoxModel
{
public:
    SearchableCombobox();
    virtual int getNumRows() override;
    virtual void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    virtual juce::String getNameForRow (int rowNumber) override;
    virtual void resized() override;
private:
    void onTextChanging(const juce::String*);
    juce::ListBox list;
    Input input;
};