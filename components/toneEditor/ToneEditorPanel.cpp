#include "ToneEditorPanel.h"
#include <vector>


ToneEditorPanel::ToneEditorPanel(I7Host* _i7Host) :
    juce::Component("tone editor panel"),
    snaPannel(_i7Host, partInfo)
{
    addAndMakeVisible(snaPannel);
}

void ToneEditorPanel::resized()
{
   
}