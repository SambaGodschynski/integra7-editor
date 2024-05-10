#pragma once

#include <juce_core/juce_core.h>
#include "Input.h"
#include <functional>
#include "LazyExecuter.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
namespace 
{
    struct SelectPopup;
}

class SearchableCombobox : public juce::Component, public juce::ListBoxModel, public LazyExecuter, public juce::Timer
{
public:
    typedef juce::Component Base;
    typedef std::vector<size_t> FilteredIndices;
    typedef std::function<size_t()> GetDataCount;
    typedef std::function<juce::String(size_t)> GetDataStringValue;
    typedef std::function<bool(size_t,const juce::String)> IsDataMatch;
    typedef std::function<void(size_t)> SelectionChanged;
    SearchableCombobox();
    virtual int getNumRows() override;
    virtual void paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected) override;
    virtual void resized() override;
    virtual void handleAsyncUpdate() override;
    virtual void timerCallback() override;
    int dropDownHeight = 120;
    virtual void mouseUp(const juce::MouseEvent& event) override;
    void setDataSource(const GetDataCount&, const GetDataStringValue&, const IsDataMatch&);
    virtual void selectedRowsChanged(int lastRowSelected) override;
    virtual void mouseWheelMove(const juce::MouseEvent&, const juce::MouseWheelDetails&) override;
private:
    juce::String searchQuery;
    FilteredIndices filteredIndices;
    GetDataCount getDataCount;
    GetDataStringValue getDataStringValue;
    IsDataMatch isDataMatch;
    SelectionChanged selectionChanged;
    std::shared_ptr<SelectPopup> selectPopup;
    void setDropDownVisible(bool);
    void onTextChanging(const juce::String*);
    void onInputClick();
    void updateFilter();
    bool containsInputAndSelection(juce::Point<int>);
    int listToSourceIndex(int);
    Input input;
    bool isDropDownVisible = false;
    int selectedIndex = -1;
    juce::String lastSearchQuery;
};
