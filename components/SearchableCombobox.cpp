#include "SearchableCombobox.h"
#include <iostream>
#include <cassert>

namespace 
{
	const int InputHeight = 30;
	const int MinHeight = InputHeight;
	const int MinSearchChars = 2;
	const float FontSize = 14.0f;
	const float CellHeight = FontSize + 1.0f;
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
        juce::Component* _parentPanel = nullptr;
        juce::Component* parentPanel();
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
		auto parentPane = parentPanel();
		auto parentBounds = parent->getBounds();
		auto bounds = parentPane->getLocalArea(parent, parentBounds);
		auto neededHeight = CellHeight * model->getNumRows() + InputHeight;
		if (neededHeight < h)
		{
			h = std::max((int)neededHeight, MinHeight);
		}
		auto appComponent = getTopLevelComponent();
		auto appBounds = appComponent->getBounds();
		auto globalBounds = appComponent->getLocalArea(parentPane, bounds);
		bool showBelow = abs(globalBounds.getY() - appBounds.getHeight()) > globalBounds.getY();
		if (showBelow)
		{
			setBounds(bounds.getX() / 2, bounds.getY() + parentBounds.getHeight(), parentBounds.getWidth(), h);
		}
		else 
		{
			setBounds(bounds.getX() / 2, bounds.getY() - h, parentBounds.getWidth(), h);
		}
		list.setBounds(0, 0, parentBounds.getWidth(), h);
		list.updateContent();
	}
	void SelectPopup::show()
	{
		parentPanel()->addAndMakeVisible(*this);
		resized();
	}
	void SelectPopup::hide()
	{
		setVisible(false);
		parentPanel()->removeChildComponent(this);
	}
	juce::Component* SelectPopup::parentPanel()
	{
		if (!_parentPanel)
		{
			auto viewport = parent->findParentComponentOfClass<juce::Viewport>();
			if (viewport)
			{
				_parentPanel = viewport->getViewedComponent();
			}
			else 
			{
				_parentPanel = parent->getTopLevelComponent();
				jassert(_parentPanel != parent); // happens when the component was not added to the app yet
			}
		}
		return _parentPanel;
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
		if (selectedIndex >= 0) 
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
	if (!event.mouseWasClicked())
	{
		return;
	}
	if(!containsInputAndSelection(event.getPosition()) && isDropDownVisible)
	{
		setDropDownVisible(false);
	}
}

bool SearchableCombobox::containsInputAndSelection(juce::Point<int> point)
{
	auto popupPos = selectPopup->getLocalPoint(this, point);
	popupPos.setX(popupPos.getX() - getX());
	if(!selectPopup->contains(popupPos) && !contains(point))
	{
		return false;
	}
	return true;
}

void SearchableCombobox::mouseWheelMove(const juce::MouseEvent& ev, const juce::MouseWheelDetails& mwd)
{
	if (!isDropDownVisible || !containsInputAndSelection(ev.getPosition()))
	{
		return;
	}
	selectPopup->list.mouseWheelMove(ev, mwd);
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
	rowIsSelected = index == selectedIndex;
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
	if (lastSearchQuery == searchQuery)
	{
		return;
	}
	lastSearchQuery = searchQuery;
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

void SearchableCombobox::selectedRowsChanged(int lastRowSelected)
{
	if (lastRowSelected < 0)
	{
		return;
	}
	bool selectionHasChanged = setSelectionIndex(lastRowSelected);
	if (!selectionHasChanged) 
	{
		return;
	}
	if (selectionChanged)
	{
		selectionChanged(selectedIndex);
	}
	doLater([this] {
		setDropDownVisible(false);
	});
}

bool SearchableCombobox::setSelectionIndex(int index)
{
	index = listToSourceIndex(index);
	if (index == selectedIndex)
	{
		return false;
	}
	selectedIndex = index;
	if (selectedIndex >= 0) 
	{
		input.setText(getDataStringValue((size_t)selectedIndex), juce::NotificationType::dontSendNotification);
	}
	return true;
}