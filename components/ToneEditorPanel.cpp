#include "ToneEditorPanel.h"
#include <vector>


// window.parent.globals.parameter.snaInstPreset
// js\view\scene_tone_sna.js
// js\view\preset\sna_inst.js

ToneEditorPanel::ToneEditorPanel() : juce::Component("tone editor panel")
{
    patchSelector.setBounds(0, 0, 300, 0);
    patchSelector.dropDownHeight = 500;
    testLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    testLabel.setEditable(true);
    testLabel.setText("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", juce::dontSendNotification);
    testLabel.setBounds(0, 60, 500, 40);
    testLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible(testLabel);
    addAndMakeVisible(patchSelector);
}