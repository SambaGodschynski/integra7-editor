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

    slider.setBounds(50, 65, 60, 300);
    slider.setSliderStyle( juce::Slider::LinearVertical );
    slider.setTextBoxStyle( juce::Slider::TextBoxAbove, false, 60, 20);
    slider.setRange(0, 127, 1);
    addAndMakeVisible(slider);


    knob.setBounds(150, 65, 160, 80);
    knob.setSliderStyle( juce::Slider::Rotary );
    knob.setTextBoxStyle( juce::Slider::TextBoxAbove, false, 60, 20);
    knob.setRange(0, 127, 1);
    addAndMakeVisible(knob);
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