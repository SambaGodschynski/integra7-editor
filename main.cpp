#include "AppTypes.h"
#include "LuaBridge.h"
#include "SysexIO.h"
#include "Rendering.h"
#include "Sidebar.h"
#include "Settings.h"
#include "CommandPalette.h"
#include "imgui_knob_image.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imcmd_command_palette.h"
#include "imsearch.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "imgui_midi_leds.h"
#include "imgui_notifications.h"
#include "SysexFileDialog.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <algorithm>


// ── Argument parsing ─────────────────────────────────────────────────────────

Args parseArguments(int argc, const char** argv)
{
    Args result;
    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        auto requireNext = [&](const char* name) -> std::string
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << name << std::endl;
                exit(-1);
            }
            return std::string(argv[i]);
        };

        if (arg == std::string("--lua-main"))
        {
            result.mainLuaFilePath = requireNext(arg);
        }
        else if (arg == std::string("--midi-in"))
        {
            result.midiInName = requireNext(arg);
        }
        else if (arg == std::string("--midi-out"))
        {
            result.midiOutName = requireNext(arg);
        }
        else if (arg == std::string("--help"))
        {
            result.printHelp = true;
        }
        else if (arg == std::string("--verbose"))
        {
            result.verbose = true;
        }        
        // unknown args are forwarded to the Lua script's Init()
    }
    return result;
}

// ── Entry point ──────────────────────────────────────────────────────────────

int main(int argc, const char** args)
{
    I7Ed ed;
    ed.args = parseArguments(argc, args);
    ed.midi.verbose = ed.args.verbose;

    // ── Early Lua init: load script and call Init(args) if defined ───────────
    const char* luaFile = ed.args.mainLuaFilePath.empty()
        ? "./lua/main.lua"
        : ed.args.mainLuaFilePath.c_str();

    ed.lua.open_libraries();
    ed.lua.new_usertype<RequestMessage>("RequestMessage",
        "sysex", &RequestMessage::sysex,
        "onMessageReceived", &RequestMessage::onMessageReceived,
        "onDone", &RequestMessage::onDone,
        "multiResponse", &RequestMessage::multiResponse,
        "receiveGapMs", &RequestMessage::receiveGapMs,
        "stopOnAddr", &RequestMessage::stopOnAddr);
    ed.lua.new_usertype<ValueChangedMessage>("ValueChangedMessage",
        "id",      &ValueChangedMessage::id,
        "i7Value", &ValueChangedMessage::i7Value);
    ed.lua["UpdateParamDisplay"] = [&ed](const std::string& id, int value)
    {
        auto* p = getParameterDef(ed, id);
        if (!p) { return; }
        p->value = (float)value;
        const auto& opts = p->optionsFn ? p->optionsFn() : p->options;
        auto it = opts.find(value);
        if (it != opts.end()) { p->stringValue = it->second; }
    };
    {
        std::string scriptDir(luaFile);
        auto slash = scriptDir.find_last_of("/\\");
        scriptDir = (slash != std::string::npos) ? scriptDir.substr(0, slash) : ".";
        auto r = ed.lua.safe_script(
            "package.path = \"" + scriptDir + "/?.lua;\" .. package.path",
            sol::script_pass_on_error);
        if (!r.valid())
        {
            std::cerr << "Lua error: " << sol::error(r).what() << std::endl;
            return -1;
        }
    }
    {
        auto r = ed.lua.safe_script_file(luaFile, sol::script_pass_on_error);
        if (!r.valid())
        {
            std::cerr << "Lua error in " << luaFile << ": "
                      << sol::error(r).what() << std::endl;
            return -1;
        }
    }

    {
        sol::protected_function initFn = ed.lua["Init"];
        if (initFn)
        {
            sol::table luaArgs = ed.lua.create_table();
            for (int i = 1; i < argc; ++i)
            {
                luaArgs[i] = std::string(args[i]);
            }
            auto r = initFn(luaArgs);
            if (!r.valid())
            {
                std::cerr << "Lua Init() error: " << sol::error(r).what() << std::endl;
                return -1;
            }
        }
    }

    if (ed.args.printHelp)
    {
        std::cout << "Allowed options:\n"
                  << "\t--lua-main <file>   replace main.lua with <file>\n"
                  << "\t--midi-in  <name>   MIDI input port name\n"
                  << "\t--midi-out <name>   MIDI output port name\n"
                  << "\t--verbose           detailed communication log\n"
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
    ed.midi.start();

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
            for (int i = 0; i < (int)names.size(); ++i)
            {
                if (names[i].find(name) != std::string::npos) { return i; }
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
        if (midiPortsData.pendingDeviceId >= 0)
        {
            ed.sidebar.deviceId = midiPortsData.pendingDeviceId;
        }

        // Command-line args override ini settings
        if (!ed.args.midiInName.empty())
        {
            int idx = findPort(ed.sidebar.inPortNames, ed.args.midiInName);
            if (idx < 0)
            {
                std::cerr << "MIDI input port not found: " << ed.args.midiInName << std::endl;
                exit(-1);
            }
            ed.sidebar.selectedInPort = idx;
            ed.midi.reopenInput(idx);
        }
        if (!ed.args.midiOutName.empty())
        {
            int idx = findPort(ed.sidebar.outPortNames, ed.args.midiOutName);
            if (idx < 0)
            {
                std::cerr << "MIDI output port not found: " << ed.args.midiOutName << std::endl;
                exit(-1);
            }
            ed.sidebar.selectedOutPort = idx;
            ed.midi.reopenOutput(idx);
        }
    }

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // ── Lua bindings (script already loaded above) ────────────────────────────
    ed.lua.set_function("GetDeviceId", [&ed]() -> int { return ed.sidebar.deviceId; });

    getDefs(ed, sections);
    ed.pSections = &sections;

    // ── Lua test API ──────────────────────────────────────────────────────────
    ed.lua.set_function("SetParam", [&ed](const std::string& id, float guiValue)
    {
        auto* p = getParameterDef(ed, id);
        if (!p) { std::cerr << "SetParam: unknown param: " << id << std::endl; return; }
        p->value = guiValue;
        valueChanged(ed, *p);
    });
    ed.lua.set_function("RequestParam", [&ed](const std::string& id)
    {
        auto* p = getParameterDef(ed, id);
        if (!p) { std::cerr << "RequestParam: unknown param: " << id << std::endl; return; }
        triggerReceive(ed, { [&ed, p]() -> std::vector<RequestMessage>
        {
            return buildParamRequests(ed, {p});
        }});
    });
    ed.lua.set_function("GetParam", [&ed](const std::string& id) -> float
    {
        auto* p = getParameterDef(ed, id);
        return p ? p->value : 0.f;
    });
    ed.lua.set_function("IsReceiving", [&ed]() -> bool
    {
        return ed.receive.active.load();
    });
    ed.lua.set_function("SendDirectMessage", [&ed](sol::table bytes)
    {
        Bytes sysex;
        for (const auto& kv : bytes)
        {
            sysex.push_back(static_cast<unsigned char>(kv.second.as<int>()));
        }
        sendMessage(ed, sysex);
    });

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
    setupCommandPalette(sections, ed);

    // ── Scrollable canvas state ───────────────────────────────────────────────
    ImVec2 scrollOfs  = {0.0f, 0.0f};
    ImVec2 canvasMax  = {0.0f, 0.0f};
    struct DragState { bool active = false; float mouseAnchor = 0, scrollAnchor = 0; };
    DragState vDrag, hDrag;
    constexpr float kSbW = 13.0f;

    ed.knobTexture      = LoadKnobTexture("assets/images/knob01.png");
    ed.sliderHandleTex  = LoadKnobTexture("assets/images/mixer_handle01.png");
    ed.drawbarTexBk     = LoadKnobTexture("assets/images/drawbar_bk.png");
    ed.drawbarTexWt     = LoadKnobTexture("assets/images/drawbar_wt.png");
    ed.drawbarTexBr     = LoadKnobTexture("assets/images/drawbar_br.png");

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
        if (ed.search.highlightTimer > 0.f)
        {
            ed.search.highlightTimer -= ImGui::GetIO().DeltaTime;
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

            // Only START a resize on a fresh click while hovering the handle and
            // while ImGui is not already processing something (e.g. a window drag).
            if (sidebarHandleHovering
                && ImGui::IsMouseClicked(0)
                && !ImGui::IsAnyItemActive())
            {
                ed.sidebar.isResizing = true;
            }

            if (sidebarHandleHovering || ed.sidebar.isResizing)
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
                if (ioRef.MouseDown[0] && ed.sidebar.isResizing)
                {
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
        processPendingReceives(ed, sections);

        // ── Lua test frame step ───────────────────────────────────────────────
        {
            sol::optional<sol::function> onFrame = ed.lua["OnFrame"];
            if (onFrame) { (*onFrame)(); }
        }

        // ── Receive progress bar ──────────────────────────────────────────────
        drawReceiveProgressBar(ed, fw, scrollOfs);

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
        renderSysexFileDialogs(ed, sections);

        // ── Render open sections ──────────────────────────────────────────────
        renderAllSections(sections, ed, canvasMax);

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
            if (!imguiActive && !ledsActive && !ed.receive.active.load())
            {
                glfwWaitEventsTimeout(1.0 / 30.0);
            }
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImCmd::RemoveAllCaches();
    ImCmd::DestroyContext();
    ImSearch::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
