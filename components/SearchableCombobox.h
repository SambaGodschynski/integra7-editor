#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Input.h"

class SearchableCombobox : public juce::Component, public juce::ListBoxModel, public juce::MouseListener
{
public:
    typedef juce::Component Base;
    SearchableCombobox();
    virtual int getNumRows() override;
    virtual void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    virtual juce::String getNameForRow (int rowNumber) override;
    virtual void resized() override;
    int dropDownHeight = 120;
    virtual void mouseUp(const juce::MouseEvent& event) override;
private:
    void setDropDownVisible(bool);
    void onTextChanging(const juce::String*);
    Component* getRoot();
    juce::ListBox list;
    Input input;
    Component* root = nullptr;
    bool isDropDownVisible = false;
};