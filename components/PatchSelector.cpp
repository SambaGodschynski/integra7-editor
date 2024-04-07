#include "PatchSelector.h"
#include <iostream>

PatchSelector::PatchSelector() : juce::Component("PatchSelector")
    , list("PatchSelector ListBox")
{
    list.setModel(this);
    list.setBounds(0, 0, getWidth(), getHeight());
    addAndMakeVisible(list);
}

void PatchSelector::resized()
{
    list.setBounds(0, 0, getWidth(), getHeight());
}

int PatchSelector::getNumRows()
{
    return 1000;
}

void PatchSelector::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    g.setFont(14.0f);
    g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour(juce::ListBox::textColourId)); 
    g.drawText (std::to_string(rowNumber), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

juce::String PatchSelector::getNameForRow (int rowNumber)
{
    return std::to_string(rowNumber);
}