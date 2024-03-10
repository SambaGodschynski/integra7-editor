#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <functional>
#include <iostream>
extern "C" {
    #include "preferences_normal_png.h"
    #include "logo_png.h"
}

#define LOCK(mutex) std::lock_guard<Mutex> guard(mutex)

PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), mainTabs(juce::TabbedButtonBar::TabsAtTop)
{
    int w = 1024, h = 768;
    setSize(w, h);

    //
    auto backgroundImage = juce::ImageCache::getFromMemory(logo_png_data, logo_png_size);
    addAndMakeVisible(background);
    background.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    background.setImage(backgroundImage);

    toneEditorPanel.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    addAndMakeVisible(mainTabs);

    mainTabs.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    
    mainTabs.addTab("Mixer", juce::Colour(), &mixerPanel, false);
    mainTabs.addTab("Tone", juce::Colour(), &toneEditorPanel, false);
}

PluginEditor::~PluginEditor()
{
}

void PluginEditor::paint(juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
}