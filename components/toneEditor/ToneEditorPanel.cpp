#include "ToneEditorPanel.h"
#include <vector>


ToneEditorPanel::ToneEditorPanel(I7Host* _i7Host) :
    juce::Component("tone editor panel"),
    snaPannel(_i7Host, partInfo)
{
    addAndMakeVisible(snaPannel);
    resized();
}

void ToneEditorPanel::resized()
{
    snaPannel.setBoundsRelative(0.0f, 0.0f, 1.0f, 1.0f);
}