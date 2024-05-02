#include "ScratchPanel.h"
#include <vector>


ScratchPanel::ScratchPanel(I7Host* _sysexSender) : 
    juce::Viewport("scratch panel"),
    p1("PRM-_SYS-_SC-NESC_TUNE", _sysexSender),
    p2("PRM-_FPART1-_PAT-_PC-RFPC_NAME", _sysexSender)
{
    p1.setBounds(100,150, 160, 200);
    p1.setSliderStyle( juce::Slider::LinearVertical );
    p1.setTextBoxStyle( juce::Slider::TextBoxAbove, false, 50, 20);
    addAndMakeVisible(p1);

    patchSelector.setBounds(0, 0, 300, 0);
    patchSelector.dropDownHeight = 500;
    patchSelector.setAlwaysOnTop(true);

    addAndMakeVisible(patchSelector);
    testLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    testLabel.setEditable(true);
    testLabel.setText("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", juce::dontSendNotification);
    testLabel.setBounds(0, 60, 500, 40);
    testLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible(testLabel);
}