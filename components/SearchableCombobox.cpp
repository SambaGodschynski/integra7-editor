#include "SearchableCombobox.h"
#include <iostream>
namespace {
	const int InputHeight = 30;
	const int MinSearchChars = 2;
	const int FontSize = 14.0f;
}

SearchableCombobox::SearchableCombobox() : juce::Component("searchable combobox")
{

	input.setColour(juce::Label::outlineColourId, juce::Colours::darkblue);
	addAndMakeVisible(input);
	input.setEditable(true);
	input.onTextChanging = std::bind(&SearchableCombobox::onTextChanging, this, std::placeholders::_1);
	list.setModel(this);
    addAndMakeVisible(list);
	list.setVisible(false);
}

void SearchableCombobox::onTextChanging(const juce::String *searchText)
{
	bool dropDownVisible  = searchText->length() > MinSearchChars;
	setDropDownVisible(dropDownVisible);
}

juce::Component* SearchableCombobox::getRoot()
{
	if (root != nullptr) {
		return root;
	}
	root = getParentComponent();
	while (root->getParentComponent() != nullptr) {
		root = root->getParentComponent();
	}
	return root;
}

void SearchableCombobox::setDropDownVisible(bool dropDownVisible)
{
	if (dropDownVisible) {

		getRoot()->addMouseListener(this, true);
	}
	if(this->isDropDownVisible && !dropDownVisible) {
		getRoot()->removeMouseListener(this);
	}
	list.setVisible(dropDownVisible);
	isDropDownVisible = dropDownVisible;
	resized();
}

void SearchableCombobox::resized()
{
	if (isDropDownVisible) {
		setBounds(0, 0, getWidth(), dropDownHeight);
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
    return 1000;
}

void SearchableCombobox::paintListBoxItem(int rowNumber, juce::Graphics &g, int width, int height, bool rowIsSelected)
{
    g.setFont(FontSize);
    g.setColour (rowIsSelected ? juce::Colours::darkblue : getLookAndFeel().findColour(juce::ListBox::textColourId)); 
    g.drawText (std::to_string(rowNumber), 2, 0, width - 4, height, juce::Justification::centredLeft, true);
}

juce::String SearchableCombobox::getNameForRow (int rowNumber)
{
    return std::to_string(rowNumber);
}

void SearchableCombobox::mouseUp(const juce::MouseEvent& event)
{
	if (!contains(event.getPosition())) {
		setDropDownVisible(false);
	}
}
