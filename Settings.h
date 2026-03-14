#pragma once

#include "AppTypes.h"
#include <vector>
#include <string>

struct OpenSectionsData
{
    std::vector<std::string>    pending;
    SectionDef::NamedSections*  pSections = nullptr;
};

struct MidiPortsData
{
    std::string   pendingInName;
    std::string   pendingOutName;
    SidebarState* pSidebar = nullptr;
    Midi*         pMidi    = nullptr;
};

void registerOpenSectionsHandler(OpenSectionsData& data);
void registerMidiPortsHandler(MidiPortsData& data);
