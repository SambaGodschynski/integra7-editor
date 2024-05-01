#pragma once

#include <juce_core/juce_core.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <vector>

class FlexContainer : public juce::Component
{
public:
    typedef std::shared_ptr<juce::Component> ChildType;
    FlexContainer(const char* name = "") : Component(name) {}
    virtual void resized() override;
    virtual void addToFlexBox(ChildType);
    juce::FlexBox& flexBox() { return _flexbox; }
private:
    juce::FlexBox _flexbox;
    typedef std::vector<ChildType> Children;
    Children _children;

};