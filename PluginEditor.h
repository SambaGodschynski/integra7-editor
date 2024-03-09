#pragma once

#include "PluginProcessor.h"
#include <memory>
#include <mutex>
#include <list>
#include <string>

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
    juce::ImageComponent background;
    std::list<std::string> logCache;
    PluginProcessor& processorRef;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
