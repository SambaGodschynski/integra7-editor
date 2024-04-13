#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "Input.h"
#include <functional>

class SearchableCombobox : public juce::Component, public juce::ListBoxModel, public juce::AsyncUpdater, public juce::Timer
{
public:
    typedef juce::Component Base;
    typedef std::vector<size_t> FilteredIndices;
    typedef std::function<size_t()> GetDataCount;
    typedef std::function<juce::String(size_t)> GetDataStringValue;
    typedef std::function<bool(size_t,const juce::String)> IsDataMatch;
    SearchableCombobox();
    virtual int getNumRows() override;
    virtual void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    virtual void resized() override;
    virtual void handleAsyncUpdate() override;
    virtual void timerCallback() override;
    int dropDownHeight = 120;
    virtual void mouseUp(const juce::MouseEvent& event) override;
    void setDataSource(const GetDataCount&, const GetDataStringValue&, const IsDataMatch&);

private:
    juce::String searchQuery;
    FilteredIndices filteredIndices;
    GetDataCount getDataCount;
    GetDataStringValue getDataStringValue;
    IsDataMatch isDataMatch;
    void setDropDownVisible(bool);
    void onTextChanging(const juce::String*);
    void onInputClick();
    void updateFilter();
    juce::ListBox list;
    Input input;
    bool isDropDownVisible = false;
};
