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

	enum StyleFlags
    {
        windowAppearsOnTaskbar                          = (1 << 0),   /**< Indicates that the window should have a corresponding
                                                                           entry on the taskbar (ignored on MacOSX) */
        windowIsTemporary                               = (1 << 1),   /**< Indicates that the window is a temporary popup, like a menu,
                                                                           tooltip, etc. */
        windowIgnoresMouseClicks                        = (1 << 2),   /**< Indicates that the window should let mouse clicks pass
                                                                           through it (may not be possible on some platforms). */
        windowHasTitleBar                               = (1 << 3),   /**< Indicates that the window should have a normal OS-specific
                                                                           title bar and frame. if not specified, the window will be
                                                                           borderless. */
        windowIsResizable                               = (1 << 4),   /**< Indicates that the window should have a resizable border. */
        windowHasMinimiseButton                         = (1 << 5),   /**< Indicates that if the window has a title bar, it should have a
                                                                           minimise button on it. */
        windowHasMaximiseButton                         = (1 << 6),   /**< Indicates that if the window has a title bar, it should have a
                                                                           maximise button on it. */
        windowHasCloseButton                            = (1 << 7),   /**< Indicates that if the window has a title bar, it should have a
                                                                           close button on it. */
        windowHasDropShadow                             = (1 << 8),   /**< Indicates that the window should have a drop-shadow (this may
                                                                           not be possible on all platforms). */
        windowRepaintedExplictly                        = (1 << 9),   /**< Not intended for public use - this tells a window not to
                                                                           do its own repainting, but only to repaint when the
                                                                           performAnyPendingRepaintsNow() method is called. */
        windowIgnoresKeyPresses                         = (1 << 10),  /**< Tells the window not to catch any keypresses. This can
                                                                           be used for things like plugin windows, to stop them interfering
                                                                           with the host's shortcut keys. */
        windowRequiresSynchronousCoreGraphicsRendering  = (1 << 11),  /**< Indicates that the window should not be rendered with
                                                                           asynchronous Core Graphics drawing operations. Use this if there
                                                                           are issues with regions not being redrawn at the expected time
                                                                           (macOS and iOS only). */
        windowIsSemiTransparent                         = (1 << 30)   /**< Not intended for public use - makes a window transparent. */

    };

	SelectPopup::SelectPopup(juce::ListBoxModel* model)
	{
		list.setModel(model);
		addAndMakeVisible(list);
		setBounds(0, 0, 800, 500);
		list.setBounds(0, 0, 800, 500);
	}
	void SelectPopup::show()
	{
		auto& lf = getLookAndFeel();
		addToDesktop (windowIsTemporary
                          | windowIgnoresKeyPresses
                          | lf.getMenuWindowFlags());
		setOpaque(true);
		setAlwaysOnTop (true);
		list.updateContent();
	}
	void SelectPopup::hide()
	{
		removeFromDesktop();
	}
}

SearchableCombobox::SearchableCombobox() : juce::Component("searchable combobox"), selectPopup(this)
{
	input.setColour(juce::Label::outlineColourId, juce::Colours::darkblue);
	addAndMakeVisible(input);
	input.setEditable(true);
	input.onTextChanging = std::bind(&SearchableCombobox::onTextChanging, this, std::placeholders::_1);
	input.onClick = std::bind(&SearchableCombobox::onInputClick, this);
	addChildComponent(selectPopup);

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
		// list.updateContent();
		input.setText("", juce::NotificationType::dontSendNotification);
		selectPopup.setVisible(true);
		selectPopup.show();
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
		selectPopup.hide();
		selectPopup.setVisible(false);
	}
	isDropDownVisible = nextVisibleState;
	resized();
	// list.setVisible(nextVisibleState);
	if (isDropDownVisible && selectedIndex > 0) {
		// list.selectRow(selectedIndex);
	}
}
#include <iostream>
void SearchableCombobox::mouseUp(const juce::MouseEvent& event)
{
	if(!selectPopup.contains(event.getPosition()) && !contains(event.getPosition()))
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
	// if (isDropDownVisible) 
	// {
	// 	auto h = dropDownHeight;
	// 	auto neededHeight = CellHeight * getNumRows() + InputHeight;
	// 	if (neededHeight < h)
	// 	{
	// 		h = std::max((int)neededHeight, MinHeight);
	// 	}
	// 	setBounds(0, 0, getWidth(), h);
	// 	childrenChanged();
	// }
	// else 
	// {
	// 	setBounds(0, 0, getWidth(), InputHeight);
	// }
	// input.setBounds(0, 0, getWidth(), InputHeight);
	// list.setBounds(0, InputHeight, getWidth(), getHeight() - InputHeight);
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
	// list.updateContent();
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
		// list.updateContent();
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
	// list.updateContent();
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