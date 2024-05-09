#include "SearchableCombobox.h"
#include <iostream>
#include <cassert>

namespace 
{
	const int InputHeight = 30;
	const int MinHeight = InputHeight;
	const int MinSearchChars = 2;
	const float FontSize = 14.0f;
	const float CellHeight = FontSize + 8.0f;
	const int DebounceTimeMillis = 500;

	struct SelectPopup : juce::Component
    {
        SelectPopup(juce::ListBoxModel*, juce::Component*);
        virtual ~SelectPopup() = default;
        void show();
        void hide();
        juce::ListBox list;
        juce::Component* parent = nullptr;
		juce::ListBoxModel* model = nullptr;
        juce::Component* _toplevelParent = nullptr;
        juce::Component* toplevelParent();
        virtual void resized() override;
    };

	SelectPopup::SelectPopup(juce::ListBoxModel* _model, juce::Component* _parent) : model(_model), parent(_parent)
	{
		list.setModel(model);
		addAndMakeVisible(list);
	}
	void SelectPopup::resized()
	{
		if (!isVisible())
		{
			return;
		}
		int h = 500;
		auto parentBounds = parent->getBounds();
		auto bounds = toplevelParent()->getLocalArea(parent, parentBounds);
		auto neededHeight = CellHeight * model->getNumRows() + InputHeight;
		if (neededHeight < h)
		{
			h = std::max((int)neededHeight, MinHeight);
		}
		setBounds(bounds.getX()/2, bounds.getY()+parentBounds.getHeight(), parentBounds.getWidth(), h);
		list.setBounds(0, 0, parentBounds.getWidth(), h);
		list.updateContent();
	}
	void SelectPopup::show()
	{
		toplevelParent()->addAndMakeVisible(*this);
		setAlwaysOnTop (true);
		resized();
	}
	void SelectPopup::hide()
	{
		setVisible(false);
		toplevelParent()->removeChildComponent(this);
	}
	juce::Component* SelectPopup::toplevelParent()
	{
		if (!_toplevelParent)
		{
			auto viewport = parent->findParentComponentOfClass<juce::Viewport>();
			if (viewport)
			{
				_toplevelParent = viewport->getViewedComponent();
			}
			else 
			{
				_toplevelParent = parent->getTopLevelComponent();
				jassert(_toplevelParent != parent); // happens when the component was not added to the app yet
			}
		}
		return _toplevelParent;
	}
}

SearchableCombobox::SearchableCombobox() : juce::Component("searchable combobox")
{
	input.setColour(juce::Label::outlineColourId, juce::Colours::darkblue);
	addAndMakeVisible(input);
	input.setEditable(true);
	input.onTextChanging = std::bind(&SearchableCombobox::onTextChanging, this, std::placeholders::_1);
	input.onClick = std::bind(&SearchableCombobox::onInputClick, this);
	selectPopup = std::make_shared<SelectPopup>(this, this);
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

void SearchableCombobox::setDropDownVisible(bool nextVisibleState)
{
	if (nextVisibleState)
	{
		searchQuery.clear();
		filteredIndices.clear();
		input.setText("", juce::NotificationType::dontSendNotification);
		selectPopup->show();
		juce::Desktop::getInstance().addGlobalMouseListener(this);
		
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
		juce::Desktop::getInstance().removeGlobalMouseListener(this);
		selectPopup->hide();
	}
	isDropDownVisible = nextVisibleState;
	resized();
	if (isDropDownVisible && selectedIndex > 0) {
		selectPopup->list.selectRow(selectedIndex);
	}
}
void SearchableCombobox::mouseUp(const juce::MouseEvent& event)
{
	if(!selectPopup->contains(event.getPosition()) && !contains(event.getPosition()))
	{
		setDropDownVisible(false);
	}
}

void SearchableCombobox::mouseWheelMove(const juce::MouseEvent& ev, const juce::MouseWheelDetails& mwd)
{
	
}

void SearchableCombobox::resized()
{
	input.setBounds(0, 0, getWidth(), getHeight());
	selectPopup->resized();
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
    g.setColour (rowIsSelected ? juce::Colours::orangered : getLookAndFeel().findColour(juce::ListBox::textColourId)); 
    g.drawText (getDataStringValue(index), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}


void SearchableCombobox::setDataSource(const GetDataCount& _getDataCount, 
	const GetDataStringValue& _getStringVal,
	const IsDataMatch& _isDataMatch)
{
	getDataCount = _getDataCount;
	getDataStringValue = _getStringVal;
	isDataMatch = _isDataMatch;
	selectPopup->list.updateContent();
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
	selectPopup->list.updateContent();
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

void SearchableCombobox::selectedRowsChanged(int lastRowSelected)
{
	if (lastRowSelected == selectedIndex)
	{
		return;
	}
	selectedIndex = listToSourceIndex(lastRowSelected);
	if (selectedIndex >= 0) 
	{
		input.setText(getDataStringValue((size_t)selectedIndex), juce::NotificationType::dontSendNotification);
	}
	doLater([this] {
		setDropDownVisible(false);
	});
}