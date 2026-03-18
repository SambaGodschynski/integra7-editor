/*
    TODO: Man in the middle mode,  midi through, handles i7 sysex
*/

#include "AppTypes.h"
#include "LuaBridge.h"
#include "SysexIO.h"
#include "Rendering.h"
#include "Sidebar.h"
#include "Settings.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imcmd_command_palette.h"
#include "imsearch.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_midi_leds.h"
#include "imgui_notifications.h"
#include "ImGuiFileDialog.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <algorithm>

namespace
{
    const float HighlightSeconds = 15.0f;
}

// ── Argument parsing ─────────────────────────────────────────────────────────

Args parseArguments(int argc, const char** argv)
{
    Args result;
    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if (std::string(arg) == "--lua-main")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.mainLuaFilePath = std::string(argv[i]);
        }
        else if (std::string(arg) == "--help")
        {
            result.printHelp = true;
        }
        else
        {
            std::cout << "unknown argument: " << arg << std::endl;
            exit(-1);
        }
    }
    return result;
}

// ── Entry point ──────────────────────────────────────────────────────────────

int main(int argc, const char** args)
{
    I7Ed ed;
    ed.args = parseArguments(argc, args);

    if (ed.args.printHelp)
    {
        std::cout << "Allowed options:\n"
                  << "\t--inputs\n"
                  << "\t--outputs\n"
                  << "\t--lua-main\n"
                  << "\t--in-portnr\n"
                  << "\t--out-portnr\n"
                  << std::endl;
        return 0;
    }

    // Enumerate ports before start() to avoid ALSA threading conflicts
    {
        int nIn  = ed.midi.getInputPortCount();
        int nOut = ed.midi.getOutputPortCount();
        for (int i = 0; i < nIn;  ++i) { ed.sidebar.inPortNames.push_back(ed.midi.getInputPortName(i)); }
        for (int i = 0; i < nOut; ++i) { ed.sidebar.outPortNames.push_back(ed.midi.getOutputPortName(i)); }
    }

    ed.midi.start();
    ed.midi.onReceive = [&ed]()
    {
        ed.midiRecvTimeNs.store(
            std::chrono::steady_clock::now().time_since_epoch().count(),
            std::memory_order_relaxed);
    };
    ed.midi.onSend = [&ed]()
    {
        ed.midiSendTimeNs.store(
            std::chrono::steady_clock::now().time_since_epoch().count(),
            std::memory_order_relaxed);
    };

    const char* luaFile = ed.args.mainLuaFilePath.empty()
        ? "./lua/main.lua"
        : ed.args.mainLuaFilePath.c_str();

    if (!glfwInit())
    {
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "i7Ed", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImCmd::CreateContext();
    ImSearch::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // ── Settings handlers ────────────────────────────────────────────────────
    SectionDef::NamedSections sections;

    OpenSectionsData openData;
    openData.pSections = &sections;
    registerOpenSectionsHandler(openData);

    MidiPortsData midiPortsData;
    midiPortsData.pSidebar = &ed.sidebar;
    midiPortsData.pMidi    = &ed.midi;
    registerMidiPortsHandler(midiPortsData);

    // Load ini so pending settings are available before getDefs
    if (io.IniFilename)
    {
        ImGui::LoadIniSettingsFromDisk(io.IniFilename);
    }

    // Apply saved MIDI port names
    {
        auto findPort = [](const std::vector<std::string>& names, const std::string& name) -> int
        {
            for (int i = 0; i < (int)names.size(); ++i)
            {
                if (names[i] == name) { return i; }
            }
            return names.empty() ? -1 : 0;
        };
        if (!midiPortsData.pendingInName.empty())
        {
            int idx = findPort(ed.sidebar.inPortNames, midiPortsData.pendingInName);
            ed.sidebar.selectedInPort = idx;
            ed.midi.reopenInput(idx);
        }
        if (!midiPortsData.pendingOutName.empty())
        {
            int idx = findPort(ed.sidebar.outPortNames, midiPortsData.pendingOutName);
            ed.sidebar.selectedOutPort = idx;
            ed.midi.reopenOutput(idx);
        }
    }

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ── Lua init ─────────────────────────────────────────────────────────────
    ed.lua.new_usertype<RequestMessage>("RequestMessage",
        "sysex", &RequestMessage::sysex,
        "onMessageReceived", &RequestMessage::onMessageReceived);
    ed.lua.new_usertype<ValueChangedMessage>("ValueChangedMessage",
        "id",      &ValueChangedMessage::id,
        "i7Value", &ValueChangedMessage::i7Value);

    ed.lua.open_libraries();
    ed.lua.script_file(luaFile);

    getDefs(ed, sections);
    ed.pSections = &sections;

    // Restore sections that were open in the previous session
    for (auto& [key, section] : sections)
    {
        for (const auto& saved : openData.pending)
        {
            if (saved == key)
            {
                section.isOpen = true;
                break;
            }
        }
    }

    // ── Command palette setup ─────────────────────────────────────────────────
    bool show_command_palette = false;

    for (auto& section : sections)
    {
        if (section.second.hideFromPalette) { continue; }
        ImCmd::Command cmd;
        cmd.Name = std::string("open ") + section.second.name;
        cmd.InitialCallback = [&section]()
        {
            section.second.isOpen = true;
        };
        ImCmd::AddCommand(std::move(cmd));
    }

    // Parameter search: "? ParamName (Section)" commands
    {
        std::unordered_map<std::string, SectionDef*> hiddenToParent;
        for (auto& [tKey, tSec] : sections)
        {
            if (tSec.tabs.empty()) { continue; }
            if (!tSec.tabCommonKey.empty())
            {
                hiddenToParent[tSec.tabCommonKey] = &sections.at(tKey);
            }
            for (const auto& tab : tSec.tabs)
            {
                for (const auto& sKey : tab.sectionKeys)
                {
                    hiddenToParent[sKey] = &sections.at(tKey);
                }
            }
        }

        std::set<std::string> seen;
        auto addParamCmds = [&](const SectionDef& sec, SectionDef* opener)
        {
            for (auto* param : sec.params)
            {
                if (!param) { continue; }
                const std::string pname = param->name();
                if (pname == HIDDEN_PARAM_NAME) { continue; }
                std::string cmdName = "? " + pname + " (" + opener->name + ")";
                if (!seen.insert(cmdName).second) { continue; }
                ImCmd::Command cmd;
                cmd.Name = std::move(cmdName);
                const std::string paramId = param->id;
                cmd.InitialCallback = [opener, paramId, &ed]()
                {
                    opener->isOpen = true;
                    ed.highlightParamId = paramId;
                    ed.highlightTimer   = HighlightSeconds;
                };
                ImCmd::AddCommand(std::move(cmd));
            }
        };

        for (auto& [key, section] : sections)
        {
            SectionDef* opener = &section;
            if (section.hideFromPalette)
            {
                auto it = hiddenToParent.find(key);
                if (it != hiddenToParent.end()) { opener = it->second; }
            }
            addParamCmds(section, opener);
            for (auto& sub : section.subSections)
            {
                addParamCmds(sub, opener);
            }
        }
    }

    // ── Scrollable canvas state ───────────────────────────────────────────────
    ImVec2 scrollOfs  = {0.0f, 0.0f};
    ImVec2 canvasMax  = {0.0f, 0.0f};
    struct DragState { bool active = false; float mouseAnchor = 0, scrollAnchor = 0; };
    DragState vDrag, hDrag;
    constexpr float kSbW = 13.0f;

    // ── Main loop ─────────────────────────────────────────────────────────────
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        const float fw = (float)display_w, fh = (float)display_h;
        const bool needV = canvasMax.y > fh;
        const bool needH = canvasMax.x > fw;
        const float maxScrollX = std::max(0.0f, canvasMax.x - fw);
        const float maxScrollY = std::max(0.0f, canvasMax.y - fh);
        const float vTrackLen  = fh - (needH ? kSbW : 0.0f);
        const float hTrackLen  = fw - (needV ? kSbW : 0.0f);
        const float vThumbLen  = needV ? vTrackLen * std::min(1.0f, fh / canvasMax.y) : 0.0f;
        const float hThumbLen  = needH ? hTrackLen * std::min(1.0f, fw / canvasMax.x) : 0.0f;
        const float vThumbTop  = (needV && maxScrollY > 0.0f)
                                 ? (vTrackLen - vThumbLen) * scrollOfs.y / maxScrollY : 0.0f;
        const float hThumbLeft = (needH && maxScrollX > 0.0f)
                                 ? (hTrackLen - hThumbLen) * scrollOfs.x / maxScrollX : 0.0f;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        // Get raw mouse position directly from GLFW (io.MousePos can be stale here)
        ImVec2 rawMouse;
        {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            const ImGuiIO& io0 = ImGui::GetIO();
            rawMouse = { (float)mx * io0.DisplayFramebufferScale.x,
                         (float)my * io0.DisplayFramebufferScale.y };
        }
        const bool rawValid = rawMouse.x >= 0.f && rawMouse.x < fw
                           && rawMouse.y >= 0.f && rawMouse.y < fh;

        const bool overVSb = needV && rawValid && rawMouse.x >= fw - kSbW;
        const bool overHSb = needH && rawValid && rawMouse.y >= fh - kSbW;
        {
            auto& ioRef = ImGui::GetIO();
            if ((overVSb || overHSb || vDrag.active || hDrag.active) && rawValid)
            {
                ioRef.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            }
            else if (rawValid)
            {
                ioRef.AddMousePosEvent(rawMouse.x + scrollOfs.x, rawMouse.y + scrollOfs.y);
            }
            else
            {
                ioRef.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            }
        }

        ImGui::NewFrame();
        if (ed.highlightTimer > 0.f)
        {
            ed.highlightTimer -= ImGui::GetIO().DeltaTime;
        }

        // ── Sidebar resize logic (no draws here) ─────────────────────────────
        bool sidebarHandleHovering = false;
        {
            const float handleX = ed.sidebar.width;
            ImGuiIO& ioRef = ImGui::GetIO();
            sidebarHandleHovering = rawMouse.x >= handleX - 4.0f
                && rawMouse.x <= handleX + 4.0f
                && rawMouse.y >= 0.0f
                && rawMouse.y <= fh;
            if (sidebarHandleHovering || ed.sidebar.isResizing)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                if (ioRef.MouseDown[0])
                {
                    ed.sidebar.isResizing = true;
                    ed.sidebar.width += ioRef.MouseDelta.x;
                    ed.sidebar.width = std::max(120.0f, std::min(600.0f, ed.sidebar.width));
                }
            }
            if (!ioRef.MouseDown[0])
            {
                ed.sidebar.isResizing = false;
            }
        }

        // Shift viewport to implement canvas scroll.
        // screen = imgui - scrollOfs from this point on.
        ImGui::GetMainViewport()->Pos     = ImVec2(scrollOfs.x, scrollOfs.y);
        ImGui::GetMainViewport()->WorkSize = {10000.f, 10000.f};
        canvasMax = {fw, fh};

        // ── Sidebar ───────────────────────────────────────────────────────────
        // Rendered AFTER viewport->Pos so ImGui's internal clamping uses the
        // current scrollOfs and the window stays pinned to screen (0,0).
        // The resize border is drawn into the sidebar's own window DrawList so
        // it shares the sidebar's z-order and section windows can appear on top.
        ImGui::SetNextWindowPos(ImVec2(scrollOfs.x, scrollOfs.y), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ed.sidebar.width, (float)display_h), ImGuiCond_Always);
        if (ImGui::Begin("##Sidebar", nullptr,
                ImGuiWindowFlags_NoMove
                | ImGuiWindowFlags_NoResize
                | ImGuiWindowFlags_NoTitleBar
                | ImGuiWindowFlags_NoBringToFrontOnFocus
                | ImGuiWindowFlags_NoCollapse))
        {
            // Draw resize border into the sidebar window's draw list.
            // PushClipRectFullScreen lets the line extend outside the sidebar bounds.
            // Because this is a window draw list (not FG), section windows rendered
            // later will appear on top of this line.
            {
                const float sx = scrollOfs.x, sy = scrollOfs.y;
                const float handleX = ed.sidebar.width;
                ImDrawList* wdl = ImGui::GetWindowDrawList();
                wdl->PushClipRectFullScreen();
                if (sidebarHandleHovering || ed.sidebar.isResizing)
                {
                    wdl->AddLine(ImVec2(handleX + sx, sy), ImVec2(handleX + sx, fh + sy),
                                 IM_COL32(180, 180, 180, 255), 2.0f);
                }
                else
                {
                    wdl->AddLine(ImVec2(handleX + sx, sy), ImVec2(handleX + sx, fh + sy),
                                 IM_COL32(80, 80, 80, 255), 1.0f);
                }
                wdl->PopClipRect();
            }
            renderSidebar(ed, sections);
        }
        ImGui::End();

        // ── Scrollbars (ForegroundDrawList) ───────────────────────────────────
        // First FG access in this frame happens here, after viewport->Pos = scrollOfs,
        // so the clip rect is [scrollOfs, scrollOfs + display].
        // All draw positions add scrollOfs to convert screen coords to imgui coords.
        {
            auto* dl = ImGui::GetForegroundDrawList();
            const float sx = scrollOfs.x, sy = scrollOfs.y;
            const float vtl2 = (needV && maxScrollY > 0.0f)
                                ? (vTrackLen - vThumbLen) * sy / maxScrollY : 0.0f;
            const float htl2 = (needH && maxScrollX > 0.0f)
                                ? (hTrackLen - hThumbLen) * sx / maxScrollX : 0.0f;
            constexpr ImU32 kTrack = IM_COL32( 30, 30, 30, 220);
            constexpr ImU32 kThumb = IM_COL32(120,120,120, 220);
            constexpr ImU32 kDrag  = IM_COL32(200,200,200, 255);
            if (needV)
            {
                dl->AddRectFilled({fw - kSbW + sx, sy},
                                  {fw + sx, fh - (needH ? kSbW : 0.f) + sy}, kTrack);
                dl->AddRectFilled({fw - kSbW + sx, vtl2 + sy},
                                  {fw + sx, vtl2 + vThumbLen + sy},
                                  vDrag.active ? kDrag : kThumb);
            }
            if (needH)
            {
                dl->AddRectFilled({sx, fh - kSbW + sy},
                                  {fw - (needV ? kSbW : 0.f) + sx, fh + sy}, kTrack);
                dl->AddRectFilled({htl2 + sx, fh - kSbW + sy},
                                  {htl2 + hThumbLen + sx, fh + sy},
                                  hDrag.active ? kDrag : kThumb);
            }
            if (needV && needH)
            {
                dl->AddRectFilled({fw - kSbW + sx, fh - kSbW + sy},
                                  {fw + sx, fh + sy}, kTrack);
            }
        }

        // ── Process pending MIDI receives ─────────────────────────────────────
        {
            for (auto it = ed.pendingReceives.begin(); it != ed.pendingReceives.end();)
            {
                {
                    std::lock_guard<std::mutex> lock(ed.pendingMutex);
                    if (it->data.empty()) { ++it; continue; }
                }
                auto msgs = it->handler(it->data);
                for (const auto& msg : msgs)
                {
                    if (!msg.id.empty()) { valueChanged(ed, msg); }
                }
                it = ed.pendingReceives.erase(it);
            }
            if (ed.pendingReceives.empty())
            {
                ed.isReceiving.store(false);
                if (ed.saveSysex.phase == I7Ed::SaveSysexState::Phase::ReadMsb)
                {
                    const std::string msbId = ed.saveSysex.partPrefix.empty()
                        ? ""
                        : [&]() -> std::string
                        {
                            int n = std::stoi(ed.saveSysex.partPrefix.substr(5, 2));
                            return "PRM-_PRF-_FP" + std::to_string(n) + "-NEFP_PAT_BS_MSB";
                        }();
                    auto* msbParam = getParameterDef(ed, msbId);
                    int msb = msbParam ? (int)msbParam->value : -1;
                    ed.saveSysex.tonePrefixes = getTonePrefixes(ed.saveSysex.partPrefix, msb);
                    std::vector<SectionDef::FGetReceiveSysex> getters;
                    for (auto& [key, sec] : sections)
                    {
                        for (const auto& pfx : ed.saveSysex.tonePrefixes)
                        {
                            if (key.size() >= pfx.size() && key.substr(0, pfx.size()) == pfx)
                            {
                                getters.push_back(makeParamOnlyGetter(ed, sec));
                                break;
                            }
                        }
                    }
                    ed.saveSysex.phase = I7Ed::SaveSysexState::Phase::ReadTone;
                    triggerReceive(ed, getters);
                }
                else if (ed.saveSysex.phase == I7Ed::SaveSysexState::Phase::ReadTone)
                {
                    ed.saveSysex.phase = I7Ed::SaveSysexState::Phase::Idle;
                    saveSysexToFile(ed);
                }
            }
        }

        // ── Indeterminate receive progress bar ────────────────────────────────
        if (ed.isReceiving.load())
        {
            const float elapsed = std::chrono::duration<float>(
                std::chrono::steady_clock::now() - ed.receiveStartTime).count();
            constexpr float kBarH   = 3.f;
            constexpr float kSegLen = 0.25f;
            constexpr float kPeriod = 1.5f;
            const float pos   = std::fmod(elapsed / kPeriod, 1.0f + kSegLen) - kSegLen;
            const float lFrac = std::clamp(pos,           0.0f, 1.0f);
            const float rFrac = std::clamp(pos + kSegLen, 0.0f, 1.0f);
            const float W     = fw;
            const float bx = scrollOfs.x, by = scrollOfs.y;
            auto* dl = ImGui::GetBackgroundDrawList();
            dl->AddRectFilled({bx,            by},
                              {W + bx,        kBarH + by},
                              IM_COL32(80, 10, 10, 200));
            dl->AddRectFilled({lFrac * W + bx, by},
                              {rFrac * W + bx, kBarH + by},
                              IM_COL32(220, 30, 30, 255));
        }

        ed.notifications.render(display_w, display_h, {scrollOfs.x, scrollOfs.y});

        if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P))
        {
            show_command_palette = !show_command_palette;
        }
        if (show_command_palette)
        {
            ImCmd::CommandPaletteWindow("CommandPalette", &show_command_palette);
        }

        // ── File dialogs ──────────────────────────────────────────────────────
        if (ImGuiFileDialog::Instance()->Display("LoadSysexDlg",
                ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                ed.loadSysex.filepath = ImGuiFileDialog::Instance()->GetFilePathName();
                loadSysexFromFile(ed);
            }
            ImGuiFileDialog::Instance()->Close();
        }

        if (ImGuiFileDialog::Instance()->Display("SaveSysexDlg",
                ImGuiWindowFlags_NoCollapse, ImVec2(600, 400)))
        {
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                ed.saveSysex.filepath = ImGuiFileDialog::Instance()->GetFilePathName();
                int n = std::stoi(ed.saveSysex.partPrefix.substr(5, 2));
                std::string msbId = "PRM-_PRF-_FP" + std::to_string(n) + "-NEFP_PAT_BS_MSB";
                SectionDef::FGetReceiveSysex msbGetter = [&ed, msbId]()
                    -> std::vector<RequestMessage>
                {
                    sol::function fn = ed.lua["CreateReceiveMessageForLeafId"];
                    sol::object obj = fn(msbId);
                    if (!obj.valid() || obj.get_type() == sol::type::nil) { return {}; }
                    return {obj.as<RequestMessage>()};
                };
                ed.saveSysex.phase = I7Ed::SaveSysexState::Phase::ReadMsb;
                triggerReceive(ed, {msbGetter});
            }
            ImGuiFileDialog::Instance()->Close();
        }

        // ── Render open sections ──────────────────────────────────────────────
        std::function<void(SectionDef&, I7Ed&)> renderSectionTree =
            [&](SectionDef& sec, I7Ed& e)
        {
            if (sec.layout == "eq3band")
            {
                renderEq3Band(sec, e);
            }
            else
            {
                renderSection(sec, e);
            }
            for (auto& sub : sec.subSections)
            {
                if (sec.accordion)
                {
                    if (ImGui::CollapsingHeader(sub.name.c_str()))
                    {
                        renderSectionTree(sub, e);
                    }
                }
                else
                {
                    renderSectionTree(sub, e);
                }
            }
        };

        for (auto& sectionPair : sections)
        {
            auto& section = sectionPair.second;
            if (!section.isOpen) { continue; }
            if (!section.tabs.empty())
            {
                renderTabbedSection(section, sections, ed, canvasMax);
            }
            else
            {
                if (ImGui::Begin(section.name.c_str(), &section.isOpen))
                {
                    {
                        ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
                        canvasMax.x = std::max(canvasMax.x, wp.x + ws.x);
                        canvasMax.y = std::max(canvasMax.y, wp.y + ws.y);
                    }
                    if (section.getReceiveSysex)
                    {
                        drawReceiveButton(ed, {section.getReceiveSysex});
                    }
                    renderSectionTree(section, ed);
                }
                ImGui::End();
            }
        }

        // ── Mouse-wheel scroll ────────────────────────────────────────────────
        if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        {
            const auto& io2 = ImGui::GetIO();
            scrollOfs.x -= io2.MouseWheelH * 30.0f;
            scrollOfs.y -= io2.MouseWheel  * 30.0f;
        }

        const bool lmb = ImGui::GetIO().MouseDown[0];

        if (vDrag.active)
        {
            if (!lmb) { vDrag.active = false; }
            else
            {
                float scrollPerPx = maxScrollY / std::max(1.0f, vTrackLen - vThumbLen);
                scrollOfs.y = std::clamp(
                    vDrag.scrollAnchor + (rawMouse.y - vDrag.mouseAnchor) * scrollPerPx,
                    0.0f, maxScrollY);
            }
        }
        else if (needV && lmb && rawValid && overVSb
                 && rawMouse.y >= vThumbTop && rawMouse.y < vThumbTop + vThumbLen)
        {
            vDrag = {true, rawMouse.y, scrollOfs.y};
        }

        if (hDrag.active)
        {
            if (!lmb) { hDrag.active = false; }
            else
            {
                float scrollPerPx = maxScrollX / std::max(1.0f, hTrackLen - hThumbLen);
                scrollOfs.x = std::clamp(
                    hDrag.scrollAnchor + (rawMouse.x - hDrag.mouseAnchor) * scrollPerPx,
                    0.0f, maxScrollX);
            }
        }
        else if (needH && lmb && rawValid && overHSb
                 && rawMouse.x >= hThumbLeft && rawMouse.x < hThumbLeft + hThumbLen)
        {
            hDrag = {true, rawMouse.x, scrollOfs.x};
        }

        scrollOfs.x = std::clamp(scrollOfs.x, 0.0f, maxScrollX);
        scrollOfs.y = std::clamp(scrollOfs.y, 0.0f, maxScrollY);

        renderMidiActivityLeds(display_w, display_h, ed.midiSendTimeNs, ed.midiRecvTimeNs,
                               {scrollOfs.x, scrollOfs.y});

        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // Idle throttle
        {
            const bool imguiActive = ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered();
            const int64_t nowNs = std::chrono::steady_clock::now().time_since_epoch().count();
            constexpr int64_t kLedFadeNs = 200'000'000LL;
            const bool ledsActive = (nowNs - ed.midiSendTimeNs.load(std::memory_order_relaxed)) < kLedFadeNs
                                 || (nowNs - ed.midiRecvTimeNs.load(std::memory_order_relaxed)) < kLedFadeNs;
            if (!imguiActive && !ledsActive && !ed.isReceiving.load())
            {
                glfwWaitEventsTimeout(1.0 / 30.0);
            }
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImCmd::DestroyContext();
    ImSearch::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
