#include "SearchableCombobox.h"
#include <iostream>
#include <cassert>

namespace {
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
}

void SearchableCombobox::onInputClick()
{
	if (isDropDownVisible)
	{
		return;
	}
	setDropDownVisible(true);
}

void SearchableCombobox::mouseWheelMove(const juce::MouseEvent& ev, const juce::MouseWheelDetails& mwd)
{

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

void SearchableCombobox::setDropDownVisible(bool nextVisibleState)
{
	if (nextVisibleState)
	{
		searchQuery.clear();
		filteredIndices.clear();
		input.setText("", juce::NotificationType::dontSendNotification);
		getTopLevelComponent()->addMouseListener(this, true);
	}
	if(!nextVisibleState)
	{
		if (selectedIndex > 0) 
		{
			input.setText(getDataStringValue((size_t)selectedIndex), juce::NotificationType::dontSendNotification);
		}
		else 
		{
			input.setText("", juce::NotificationType::dontSendNotification);
		}
		getTopLevelComponent()->removeMouseListener(this);
	}
	isDropDownVisible = nextVisibleState;
	resized();
}

void SearchableCombobox::resized()
{
	input.setBounds(0, 0, getWidth(), getHeight());
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
}

void SearchableCombobox::handleAsyncUpdate()
{
	LazyExecuter::handleAsyncUpdate();
	updateFilter();
}

void SearchableCombobox::updateFilter()
{
	filteredIndices.clear();
	if (searchQuery.isEmpty())
	{
		resized();
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
}

void SearchableCombobox::timerCallback()
{
	triggerAsyncUpdate();
	stopTimer();
}

int SearchableCombobox::listToSourceIndex(int listIndex)
{
	if (listIndex < 0) 
	{
		return -1;
	}
	if (filteredIndices.empty()) 
	{
		return listIndex;
	}
	return filteredIndices.at(listIndex);
}