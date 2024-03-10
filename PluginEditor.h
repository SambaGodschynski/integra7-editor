#pragma once

#include "PluginProcessor.h"
#include <memory>
#include <mutex>
#include <list>
#include <string>
#include "components/MixerPanel.h"
#include "components/ToneEditorPanel.h"

//==============================================================================
class PluginEditor  : public juce::AudioProcessorEditor
{
public:
    explicit PluginEditor (PluginProcessor&);
    virtual ~PluginEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    juce::TabbedComponent mainTabs;
    MixerPanel mixerPanel;
    ToneEditorPanel toneEditorPanel;
    juce::ImageComponent background;
    std::list<std::string> logCache;
    PluginProcessor& processorRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
