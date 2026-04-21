#pragma once

#include "AppTypes.h"

void openSaveSysexDialog(const std::string& partPrefix, I7Ed& ed);
void openLoadSysexDialog();
void renderSysexFileDialogs(I7Ed& ed, SectionDef::NamedSections& sections);
