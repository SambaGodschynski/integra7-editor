#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <components/I7Host.h>
#include "sna/SnaPanel.h"
#include <components/PartInfo.h>

class ToneEditorPanel : public juce::Component
{
public:
    ToneEditorPanel(I7Host*);
    virtual void resized() override;
private:
    PartInfo partInfo;
    ted_sna::SnaPanel snaPannel;
};