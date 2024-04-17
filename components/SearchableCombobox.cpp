#include "SearchableCombobox.h"
#include <iostream>
#include <cassert>

namespace {
	const int InputHeight = 30;
	const int MinHeight = InputHeight;
	const int MinSearchChars = 2;
	const float FontSize = 14.0f;
	const float CellHeight = FontSize + 8.0f;
	const int DebounceTimeMillis = 500;
}

SearchableCombobox::SearchableCombobox() : juce::Component("searchable combobox")
{

	input.setColour(juce::Label::outlineColourId, juce::Colours::darkblue);
	addAndMakeVisible(input);
	input.setEditable(true);
	input.onTextChanging = std::bind(&SearchableCombobox::onTextChanging, this, std::placeholders::_1);
	input.onClick = std::bind(&SearchableCombobox::onInputClick, this);
	list.setModel(this);
    addAndMakeVisible(list);
	list.setVisible(false);
}

void SearchableCombobox::onInputClick()
{
	if (isDropDownVisible)
	{
		return;
	}
	setDropDownVisible(true);
}

void SearchableCombobox::onTextChanging(const juce::String *_searchQuery)
{
	searchQuery = *_searchQuery;
	if (isTimerRunning())
	{
		stopTimer();
	}
	startTimer(DebounceTimeMillis);
}

void SearchableCombobox::setDropDownVisible(bool dropDownVisible)
{
	if (dropDownVisible)
	{

		getTopLevelComponent()->addMouseListener(this, true);
	}
	if(this->isDropDownVisible && !dropDownVisible)
	{
		getTopLevelComponent()->removeMouseListener(this);
	}
	list.setVisible(dropDownVisible);
	isDropDownVisible = dropDownVisible;
	resized();
}

void SearchableCombobox::resized()
{
	if (isDropDownVisible) {
		auto h = dropDownHeight;
		auto neededHeight = CellHeight * getNumRows() + InputHeight;
		if (neededHeight < h)
		{
			h = std::max((int)neededHeight, MinHeight);
		}
		setBounds(0, 0, getWidth(), h);
		childrenChanged();
	}
	else {
		setBounds(0, 0, getWidth(), InputHeight);
	}
	input.setBounds(0, 0, getWidth(), InputHeight);
	list.setBounds(0, InputHeight, getWidth(), getHeight() - InputHeight);
}

int SearchableCombobox::getNumRows()
{
	if(!getDataCount)
	{
		return 0;
	}
	if (searchQuery.isEmpty())
	{
    	return getDataCount();
	}
	return filteredIndices.size();
}

void SearchableCombobox::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
	size_t index = (size_t)rowNumber;
	if (!filteredIndices.empty())
	{
		index = filteredIndices.at(index);
	}
    g.setFont(FontSize);
    g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour(juce::ListBox::textColourId)); 
    g.drawText (getDataStringValue(index), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

void SearchableCombobox::mouseUp(const juce::MouseEvent& event)
{
	if (!contains(event.getPosition())) {
		setDropDownVisible(false);
	}
}

void SearchableCombobox::setDataSource(const GetDataCount& _getDataCount, 
	const GetDataStringValue& _getStringVal,
	const IsDataMatch& _isDataMatch)
{
	getDataCount = _getDataCount;
	getDataStringValue = _getStringVal;
	isDataMatch = _isDataMatch;
	list.updateContent();
}

void SearchableCombobox::handleAsyncUpdate()
{
	updateFilter();
}

void SearchableCombobox::updateFilter()
{
	filteredIndices.clear();
	if (searchQuery.isEmpty())
	{
		resized();
		list.updateContent();
		return;
	}
	size_t count = getDataCount();
	filteredIndices.reserve(count);
	for(size_t i=0; i<count; ++i)
	{
		if (isDataMatch(i, searchQuery))
		{
			filteredIndices.push_back(i);
		}
	}
	resized();
	list.updateContent();
}

void SearchableCombobox::timerCallback()
{
	triggerAsyncUpdate();
	stopTimer();
}