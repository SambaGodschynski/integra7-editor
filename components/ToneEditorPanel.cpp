#include "ToneEditorPanel.h"

ToneEditorPanel::ToneEditorPanel() : juce::Component("tone editor panel")
{
    patchSelector.setBounds(0, 0, 300, 300);
    addAndMakeVisible(patchSelector);
}