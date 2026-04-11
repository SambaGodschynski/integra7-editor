#pragma once

#include "AppTypes.h"
#include "imgui.h"
#include <vector>

void renderCombo(ParameterDef& param, I7Ed& ed);
void renderSection(SectionDef& section, I7Ed& ed);
void renderEq3Band(SectionDef& section, I7Ed& ed);
void renderKeyboard(SectionDef& section, I7Ed& ed);
void renderRssXY(SectionDef& section, I7Ed& ed);
void renderMixer(SectionDef& section, I7Ed& ed);
void renderTabbedSection(SectionDef& section, SectionDef::NamedSections& sections,
                         I7Ed& ed, ImVec2& canvasMax);
void drawReceiveButton(I7Ed& ed, const std::vector<SectionDef::FGetReceiveSysex>& getters);
