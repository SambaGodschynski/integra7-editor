#include "ToneEditorPanel.h"

ToneEditorPanel::ToneEditorPanel() : juce::Component("tone editor panel")
{
    patchSelector.setBounds(0, 0, 300, 500);
    testLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    testLabel.setText("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", juce::dontSendNotification);
    testLabel.setBounds(0, 60, 500, 40);
    testLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible(testLabel);
    addAndMakeVisible(patchSelector);
}