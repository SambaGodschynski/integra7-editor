#include "Sidebar.h"
#include "Rendering.h"
#include "SysexIO.h"
#include "imgui.h"
#include <sol/sol.hpp>
#include <cstdio>
#include <functional>

// ── Internal helpers ─────────────────────────────────────────────────────────

static ToneType getPartToneType(I7Ed& ed, int partNr)
{
    std::string id = "PRM-_PRF-_FP" + std::to_string(partNr) + "-NEFP_TYPE_DUMMY";
    ParameterDef* p = getParameterDef(ed, id);
    if (!p) { return ToneType::Unknown; }
    int v = (int)p->value;
    if (v < 1 || v > 5) { return ToneType::Unknown; }
    return static_cast<ToneType>(v);
}

static void renderPartButtons(SectionDef::NamedSections& sections, int partNr, ToneType type)
{
    char pn[4];
    snprintf(pn, sizeof(pn), "%02d", partNr);
    std::string base = std::string("Part ") + pn + " ";

    auto viewButton = [&](const char* label, const std::string& key)
    {
        auto it = sections.find(key);
        if (it == sections.end()) { return; }
        SectionDef& sec = it->second;
        float btnWidth = ImGui::CalcTextSize(label).x
            + ImGui::GetStyle().FramePadding.x * 2.0f;
        if (ImGui::GetContentRegionAvail().x < btnWidth + 4.0f)
        {
            ImGui::NewLine();
        }
        bool wasOpen = sec.isOpen;
        if (wasOpen)
        {
            ImGui::PushStyleColor(ImGuiCol_Button,
                ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
        }
        if (ImGui::SmallButton(label))
        {
            sec.isOpen = !sec.isOpen;
        }
        if (wasOpen)
        {
            ImGui::PopStyleColor();
        }
        ImGui::SameLine();
    };

    // Part sections (always visible)
    viewButton("Common",   base + "Common");
    viewButton("EQ",       base + "EQ");
    viewButton("Keys",     base + "Keyboard");
    viewButton("Pitch",    base + "Pitch");
    viewButton("Offset",   base + "Offset");
    viewButton("Scale",    base + "Scale");
    viewButton("MIDI",     base + "MIDI");
    ImGui::NewLine();

    // Tone-type specific sections
    switch (type)
    {
        case ToneType::SNA:
        {
            viewButton("SNA", base + "SNA");
            viewButton("MFX", base + "SN-A MFX");
            break;
        }
        case ToneType::SNS:
        {
            std::string pfx = base + "SN-S ";
            viewButton("SN-S",   pfx + "Common");
            viewButton("Tone",   pfx + "Tone");
            viewButton("MFX",    pfx + "MFX");
            break;
        }
        case ToneType::SND:
        {
            std::string pfx = base + "SN-D ";
            viewButton("Instrument", pfx + "Instrument");
            viewButton("CompEq",     pfx + "CompEq");
            viewButton("MFX",        pfx + "MFX");
            break;
        }
        case ToneType::PCMS:
        {
            std::string pfx = base + "PCM-S ";
            viewButton("PCM-S",  pfx + "Common");
            viewButton("Tone",   pfx + "Tone");
            viewButton("MFX",    pfx + "MFX");
            break;
        }
        case ToneType::PCMD:
        {
            std::string pfx = base + "PCM-D ";
            viewButton("PCM-D",  pfx + "Common");
            viewButton("PCM Pitch", pfx + "Pitch");
            viewButton("WMT",    pfx + "WMT");
            viewButton("CompEq", pfx + "CompEq");
            viewButton("MFX",    pfx + "MFX");
            break;
        }
        default:
        {
            ImGui::TextDisabled("(unknown)");
            break;
        }
    }
    ImGui::NewLine();
}

static void renderMidiPortCombo(
    const char* id,
    const std::vector<std::string>& names,
    int& selected,
    std::function<void(int)> onSelect)
{
    const char* preview = (selected >= 0 && selected < (int)names.size())
        ? names[selected].c_str()
        : "(none)";
    ImGui::SetNextItemWidth(-1.0f);
    if (ImGui::BeginCombo(id, preview))
    {
        for (int i = 0; i < (int)names.size(); ++i)
        {
            bool sel = (i == selected);
            if (ImGui::Selectable(names[i].c_str(), sel))
            {
                selected = i;
                onSelect(i);
            }
        }
        ImGui::EndCombo();
    }
}

// ── Public function ──────────────────────────────────────────────────────────

void renderSidebar(I7Ed& ed, SectionDef::NamedSections& sections)
{
    // ── Global section buttons ────────────────────────────────────────────────
    {
        auto sectionButton = [&](const char* label, const char* key)
        {
            auto it = sections.find(key);
            if (it == sections.end()) { return; }
            SectionDef& sec = it->second;
            float btnWidth = ImGui::CalcTextSize(label).x
                + ImGui::GetStyle().FramePadding.x * 2.0f;
            if (ImGui::GetContentRegionAvail().x < btnWidth + 4.0f)
            {
                ImGui::NewLine();
            }
            bool wasOpen = sec.isOpen;
            if (wasOpen)
            {
                ImGui::PushStyleColor(ImGuiCol_Button,
                    ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
            }
            if (ImGui::SmallButton(label)) { sec.isOpen = !sec.isOpen; }
            if (wasOpen) { ImGui::PopStyleColor(); }
            ImGui::SameLine();
        };

        sectionButton("Mixer",        "Mixer");
        sectionButton("Presets",      "presets");
        sectionButton("Tone Mgmt",    "Tone Management");
        sectionButton("Studio Mgmt",  "Studio Management");
        sectionButton("Effects",      "Studio Set Effects");
        sectionButton("RSS",          "rss");
        sectionButton("Expansions",   "Expansion Slots");
        sectionButton("System",       "System");
        ImGui::NewLine();
    }
    ImGui::Separator();

    // ── MIDI settings ─────────────────────────────────────────────────────────
    if (ImGui::CollapsingHeader("MIDI", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::TextUnformatted("Device ID");
        {
            char preview[12];
            snprintf(preview, sizeof(preview), "%d", ed.sidebar.deviceId + 1);
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##DeviceId", preview))
            {
                for (int i = 16; i < 32; ++i)
                {
                    char label[8];
                    snprintf(label, sizeof(label), "%d", i + 1);
                    bool sel = (i == ed.sidebar.deviceId);
                    if (ImGui::Selectable(label, sel))
                    {
                        ed.sidebar.deviceId = i;
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::TextUnformatted("In");
        renderMidiPortCombo("##MidiIn", ed.sidebar.inPortNames,
            ed.sidebar.selectedInPort,
            [&](int i) { ed.midi.reopenInput(i); });
        ImGui::TextUnformatted("Out");
        renderMidiPortCombo("##MidiOut", ed.sidebar.outPortNames,
            ed.sidebar.selectedOutPort,
            [&](int i) { ed.midi.reopenOutput(i); });
    }
    ImGui::Separator();

    for (int partIdx = 0; partIdx < 16; ++partIdx)
    {
        int partNr = partIdx + 1;
        char pn[4];
        snprintf(pn, sizeof(pn), "%02d", partNr);
        std::string headerLabel = std::string("Part ") + pn;

        ImGui::PushID(partIdx);
        bool partOpen = ImGui::CollapsingHeader(headerLabel.c_str());
        if (ImGui::IsItemToggledOpen())
        {
            sol::optional<sol::function> buildFn = ed.lua["BuildSyncToneTypeRequest"];
            if (buildFn)
            {
                sol::object obj = (*buildFn)(partNr);
                if (obj.valid() && obj.get_type() != sol::type::nil)
                {
                    RequestMessage req = obj.as<RequestMessage>();
                    if (!ed.receive.active.exchange(true))
                    {
                        ed.receive.startTime = std::chrono::steady_clock::now();
                        ed.receive.lastActivityNs.store(
                            ed.receive.startTime.time_since_epoch().count(),
                            std::memory_order_relaxed);
                        enqueueRequest(ed, req);
                        ++ed.receive.totalCount;
                    }
                }
            }
        }
        if (partOpen)
        {
            std::string typeParamId = "PRM-_PRF-_FP"
                + std::to_string(partNr)
                + "-NEFP_TYPE_DUMMY";
            ParameterDef* typeParam = getParameterDef(ed, typeParamId);
            if (typeParam != nullptr)
            {
                float comboWidth = ImGui::CalcTextSize("PCM-D").x
                    + ImGui::GetStyle().FramePadding.x * 2.0f
                    + ImGui::GetFrameHeight();
                ImGui::SetNextItemWidth(comboWidth);
                renderCombo(*typeParam, ed);
            }
            ToneType type = getPartToneType(ed, partNr);
            renderPartButtons(sections, partNr, type);
        }
        ImGui::PopID();
    }
}
