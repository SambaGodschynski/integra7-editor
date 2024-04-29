#include "I7Parameter.h"
#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <PluginEditor.h>

ISysexSender* getSysexSender(const juce::Component* component)
{
    auto* ed = component->findParentComponentOfClass<PluginEditor>();
}