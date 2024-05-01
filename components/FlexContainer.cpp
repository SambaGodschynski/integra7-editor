#include "FlexContainer.h"

void FlexContainer::resized()
{
	auto w = getWidth();
	auto h = getHeight();
	auto bounds = juce::Rectangle<int>(0, 0, w, h);
	_flexbox.performLayout(bounds);
}

void FlexContainer::addToFlexBox(ChildType component)
{
	_children.push_back(component);
	addAndMakeVisible(component.get());
	juce::FlexItem flexItem(component->getWidth(), component->getHeight(), *component);
	flexItem.alignSelf = juce::FlexItem::AlignSelf::stretch;
	_flexbox.items.add(flexItem);
	resized();
}