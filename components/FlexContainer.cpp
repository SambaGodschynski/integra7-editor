#include "FlexContainer.h"

void FlexContainer::resized()
{
	auto w = getWidth();
	auto h = getHeight();
	auto bounds = juce::Rectangle<int>(0, 0, w, h);
	_flexbox.performLayout(bounds);
}

juce::FlexItem* FlexContainer::addToFlexBox(ChildType component)
{
	_children.push_back(component);
	addAndMakeVisible(component.get());
	juce::FlexItem flexItem(component->getWidth(), component->getHeight(), *component);
	_flexbox.items.add(flexItem);
	resized();
	return &(_flexbox.items.getReference(_flexbox.items.size()-1));
}