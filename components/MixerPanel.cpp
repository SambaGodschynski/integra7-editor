#include "MixerPanel.h"

MixerPanel::MixerPanel() : juce::Component("mixer panel")
{
    flexBox.flexDirection = juce::FlexBox::Direction::row;
    flexBox.flexWrap = juce::FlexBox::Wrap::noWrap;
	flexBox.alignContent = juce::FlexBox::AlignContent::flexStart;
	flexBox.alignItems = juce::FlexBox::AlignItems::flexStart;
	flexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    for(size_t i=0; i<NumChannelStrips; ++i)
    {
        auto& channelStrip = channelStrips[i];
        juce::FlexItem flexItem((float)channelStrip.getWidth(), (float)channelStrip.getHeight(), channelStrip);
        flexItem.margin = juce::FlexItem::Margin(5);
        flexBox.items.add(flexItem);
        addAndMakeVisible(channelStrip);
    }
    resized();
}

void MixerPanel::resized()
{
    auto bounds = juce::Rectangle<int>(0, 0, getWidth(), getHeight());
	flexBox.performLayout(bounds);
}