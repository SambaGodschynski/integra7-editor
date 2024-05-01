#include "MixerChannelStrip.h"

MixerChannelStrip::MixerChannelStrip() : juce::Component("mixer channel strip")
{
    flexBox.flexDirection = juce::FlexBox::Direction::column;
	//flexBox.flexWrap = juce::FlexBox::Wrap::noWrap;
	// flexBox.alignContent = juce::FlexBox::AlignContent::flexStart;
	// flexBox.alignItems = juce::FlexBox::AlignItems::flexStart;
	// flexBox.justifyContent = juce::FlexBox::JustifyContent::flexStart;

    setSize(50, 500);
    volume.setSize(50, 200);
    volume.setSliderStyle( juce::Slider::LinearVertical );
    volume.setTextBoxStyle( juce::Slider::TextBoxAbove, false, 30, 20);
    volume.setRange(0, 127, 1);
    volume.setValue(127);

    {
        juce::FlexItem flexItem((float)volume.getWidth(), (float)volume.getHeight(), volume);
        flexBox.items.add(flexItem);
        addAndMakeVisible(volume);
    }

    pan.setSize(50, 80);
    pan.setSliderStyle( juce::Slider::Rotary );
    pan.setTextBoxStyle( juce::Slider::TextBoxAbove, false, 30, 20);
    pan.setRange(-127/2, 127/2, 1);
    pan.setValue(0);

    {
        juce::FlexItem flexItem((float)pan.getWidth(), (float)pan.getHeight(), pan);
        flexItem.margin = juce::FlexItem::Margin(10, 0, 0, 0);
        flexBox.items.add(flexItem);
        addAndMakeVisible(pan);
    }
    resized();
}

void MixerChannelStrip::resized()
{
    auto bounds = juce::Rectangle<int>(0, 0, getWidth(), getHeight());
	flexBox.performLayout(bounds);
}