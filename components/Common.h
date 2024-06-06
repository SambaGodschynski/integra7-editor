#pragma once

#ifdef DEBUG
    #define DEBUGONLY(x) (x)
#else
    #define DEBUGONLY(x)
#endif

#define I7Alert(msg) juce::AlertWindow::showMessageBox(juce::MessageBoxIconType::WarningIcon, "Request Failed", (msg))