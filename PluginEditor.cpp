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
    : AudioProcessorEditor (&p), processorRef (p)
{
    int w = 800, h = 600;
    setSize(w, h);

    //
    auto backgroundImage = juce::ImageCache::getFromMemory(logo_png_data, logo_png_size);
    background.setBounds(0, 65, 800, 600);
    background.setImage(backgroundImage);
    addAndMakeVisible(background);

    juce::Slider
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