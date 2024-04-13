#include "Input.h"
#include <iostream>


Input::Input()
{

}

void Input::textEditorTextChanged(juce::TextEditor& ed)
{
	if(onTextChanging) {
        auto txt =  ed.getText();
        onTextChanging(&txt);
    }
}

void Input::mouseDown(const juce::MouseEvent& event)
{
    if (onClick) {
        onClick();
    }
}