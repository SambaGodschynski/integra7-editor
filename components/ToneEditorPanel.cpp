#include "ToneEditorPanel.h"
#include <vector>

namespace 
{
    std::vector<std::string> test_data;
}


ToneEditorPanel::ToneEditorPanel() : juce::Component("tone editor panel")
{
    for(int i=0; i<1000; ++i)
    {
        test_data.push_back(std::string("Hello Nr.") + std::to_string(i));
    }
    
    patchSelector.setDataSource(
        []() { return test_data.size(); }, 
        [](size_t index) { return juce::String(test_data.at(index)); },
        [](size_t index, const juce::String &str)
        {
            return test_data.at(index).find(str.toRawUTF8()) != std::string::npos;
        }
    );

    patchSelector.setBounds(0, 0, 300, 0);
    patchSelector.dropDownHeight = 500;
    testLabel.setFont (juce::Font (16.0f, juce::Font::bold));
    testLabel.setEditable(true);
    testLabel.setText("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX", juce::dontSendNotification);
    testLabel.setBounds(0, 60, 500, 40);
    testLabel.setColour (juce::Label::textColourId, juce::Colours::orange);
    addAndMakeVisible(testLabel);
    addAndMakeVisible(patchSelector);
}