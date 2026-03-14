#include "Settings.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <cstring>

void registerOpenSectionsHandler(OpenSectionsData& data)
{
    ImGuiSettingsHandler h = {};
    h.TypeName  = "I7EdOpenSections";
    h.TypeHash  = ImHashStr("I7EdOpenSections");
    h.UserData  = &data;
    h.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char*) -> void*
    {
        return (void*)1;
    };
    h.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, void*, const char* line)
    {
        auto* d = static_cast<OpenSectionsData*>(handler->UserData);
        if (line[0] != '\0')
        {
            d->pending.push_back(line);
        }
    };
    h.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
        auto* d = static_cast<OpenSectionsData*>(handler->UserData);
        buf->appendf("[%s][]\n", handler->TypeName);
        for (const auto& [key, sec] : *d->pSections)
        {
            if (sec.isOpen)
            {
                buf->appendf("%s\n", key.c_str());
            }
        }
        buf->appendf("\n");
    };
    ImGui::AddSettingsHandler(&h);
}

void registerMidiPortsHandler(MidiPortsData& data)
{
    ImGuiSettingsHandler h = {};
    h.TypeName  = "I7EdMidiPorts";
    h.TypeHash  = ImHashStr("I7EdMidiPorts");
    h.UserData  = &data;
    h.ReadOpenFn = [](ImGuiContext*, ImGuiSettingsHandler*, const char*) -> void*
    {
        return (void*)1;
    };
    h.ReadLineFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, void*, const char* line)
    {
        auto* d = static_cast<MidiPortsData*>(handler->UserData);
        if (strncmp(line, "In=",  3) == 0) { d->pendingInName  = line + 3; }
        if (strncmp(line, "Out=", 4) == 0) { d->pendingOutName = line + 4; }
    };
    h.WriteAllFn = [](ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
        auto* d = static_cast<MidiPortsData*>(handler->UserData);
        SidebarState& sb = *d->pSidebar;
        buf->appendf("[%s][]\n", handler->TypeName);
        if (sb.selectedInPort >= 0 && sb.selectedInPort < (int)sb.inPortNames.size())
        {
            buf->appendf("In=%s\n", sb.inPortNames[sb.selectedInPort].c_str());
        }
        if (sb.selectedOutPort >= 0 && sb.selectedOutPort < (int)sb.outPortNames.size())
        {
            buf->appendf("Out=%s\n", sb.outPortNames[sb.selectedOutPort].c_str());
        }
        buf->appendf("\n");
    };
    ImGui::AddSettingsHandler(&h);
}
