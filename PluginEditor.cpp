#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <functional>
#include <iostream>

#define LOCK(mutex) std::lock_guard<Mutex> guard(mutex)


#ifdef WIN32
    #include <windows.h>
    struct Console {
        Console();
        ~Console();
    };
    Console::Console() {
        AllocConsole();
        freopen("conin$", "r", stdin);
        freopen("conout$", "w", stdout);
        freopen("conout$", "w", stderr);
        HWND consoleHandle = GetConsoleWindow();
        MoveWindow(consoleHandle, 1, 1, 680, 480, 1);
        std::cout << "Console initalized." << std::endl;
    }
    Console::~Console() {
        FreeConsole();
    }


    //#ifdef _DEBUG
    Console console;
    //#endif
#endif


PluginEditor::PluginEditor(PluginProcessor& p)
    : AudioProcessorEditor(&p), 
    processorRef(p), 
    mainTabs(juce::TabbedButtonBar::TabsAtTop),
    toneEditorPanel(&p),
    scratchPanel(&p)
{
    int w = 1024, h = 768;
    setSize(w, h);
    setResizable(true, true);
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