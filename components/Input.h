
#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

class Input : public juce::Label
{
public:
    Input();
    std::function<void(const juce::String*)> onTextChanging; 
    virtual void focusGained(juce::Component::FocusChangeType cause) override;
protected:
    virtual void textEditorTextChanged(juce::TextEditor&) override;
};