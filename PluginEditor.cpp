#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <functional>
#include <iostream>

#define LOCK(mutex) std::lock_guard<Mutex> guard(mutex)

PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), 
    processorRef(p), 
    mainTabs(juce::TabbedButtonBar::TabsAtTop),
    toneEditorPanel(&p),
    scratchPanel(&p)
{
    int w = 1024, h = 768;
    setSize(w, h);
    setResizable(true, false);
    //
    addAndMakeVisible(mainTabs);
    mainTabs.addTab("Mixer", juce::Colour(), &mixerPanel, false);
    mainTabs.addTab("ToneEditor", juce::Colour(), &toneEditorPanel, false);
    mainTabs.addTab("Scratch", juce::Colour(), &scratchPanel, false);
    resized();
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
    mixerPanel.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    scratchPanel.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    toneEditorPanel.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
    mainTabs.setBoundsRelative(0.f, 0.f, 1.f, 1.f);
}