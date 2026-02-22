/*
    TODO: Man in the middle mode,  midi through, handles i7 sysex 
*/

#include "imgui.h"
#include "imgui-knobs.h"
#include "imcmd_command_palette.h"
#include "imsearch.h"
#include "imgui_toggle.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <sol/sol.hpp>
#include <iostream>
#include <rtmidi/RtMidi.h>
#include <string>
#include "ParameterDef.h"
#include "imgui_envelope.h"
#include "imgui_notifications.h"
#include <vector>
#include <list>
#include <tuple>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <memory>

#define HIDDEN_PARAM_NAME "__HIDDEN__"

struct Args 
{
    bool listOutputs = false;
    bool listInputs = false;
    bool printHelp = false;
    std::string mainLuaFilePath;
    int inPortNr = -1;
    int outPortNr = -1;
};

struct PendingReceive
{
    RequestMessage::FOnMessageReceived handler;
    Bytes data;
};

struct I7Ed
{
    RtMidiIn  midiIn;
    RtMidiOut midiOut;
    Args args;
    sol::state lua;
    std::unordered_map<std::string, std::shared_ptr<ParameterDef>> parameterDefs;
    // async receive
    std::atomic<bool> isReceiving{false};
    std::chrono::steady_clock::time_point receiveStartTime;
    std::mutex pendingMutex;
    std::vector<PendingReceive> pendingReceives;
    std::thread receiveThread;
    NotificationQueue notifications;
};

Args parseArguments(int argc, const char **argv)
{
    Args result;
    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];
        if (std::string(arg) == "--outputs")
        {
            result.listOutputs = true;
        }
        else if (std::string(arg) == "--inputs")
        {
            result.listInputs = true;
        }
        else if (std::string(arg) == "--lua-main")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.mainLuaFilePath = std::string(argv[i]);
            continue;
        }
        else if (std::string(arg) == "--in-portnr")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.inPortNr = (int)atoi(argv[i]);
            continue;
        }
        else if (std::string(arg) == "--out-portnr")
        {
            ++i;
            if (i >= argc)
            {
                std::cerr << "missing argument for " << arg << std::endl;
                exit(-1);
            }
            result.outPortNr = (int)atoi(argv[i]);
            continue;
        }
        else if (std::string(arg) == "--help")
        {
            result.printHelp = true;
        }
        else 
        {
            std::cout << "unkown argument: " << arg << std::endl;
            exit(-1);
        }
    }
    return result;
}


// BEGIN: MIDI IO
template<class TMidiIO>
void printMidiIo()
{
    TMidiIO midiIo;
    auto nIos = midiIo.getPortCount();
    for (size_t idx = 0; idx < nIos; ++idx)
    {
        std::cout << (idx) << ": " << midiIo.getPortName(idx) << std::endl;
    }
}

template<class TMidiIO>
void openPort(TMidiIO &port, size_t portNr)
{
    auto nIos = port.getPortCount();
    if (portNr >= nIos)
    {
        std::cerr << "invalid port number: " << portNr << std::endl;
        exit(1);
    } 
    port.openPort(portNr);
    std::cout << "open: (" << portNr << ") " << port.getPortName(portNr) << std::endl;
}

void sendMessage(RtMidiOut &midiOut, const Bytes& message)
{
    midiOut.sendMessage(&message);
}
// END: MIDI IO

// BEGIN: LUA to DEF. TODO Refactor into separate file
template<typename T>
T require_key(sol::table tbl, const std::string& key) {
    sol::optional<T> val = tbl[key];
    if (!val) {
        throw std::runtime_error("Lua table is missing required key: " + key);
    }
    return val.value();
}

template<typename T>
T optional_key(sol::table tbl, const std::string& key, const T& defaultValue) {
    sol::optional<T> val = tbl[key];
    if (!val.has_value()) {
       return defaultValue;
    }
    return val.value();
}

std::string getDefaultValue(const ParameterDef::SelectionOptions &options, int defaultValue)
{
    const auto& it = options.find(defaultValue);
    if (it == options.end())
    {
        return std::string();
    }
    return it->second;
}

void getSection(I7Ed &ed, sol::table &lua_table, SectionDef &outSectionDef)
{
    outSectionDef.name = require_key<std::string>(lua_table, "name");
    outSectionDef.getReceiveSysex = optional_key<SectionDef::FGetReceiveSysex>(lua_table, "getReceiveValueSysex", nullptr);
    if (lua_table["params"] != sol::nil) 
    {
        sol::table params = lua_table["params"];
        for(auto &luaParamPair : params)
        {
            auto param = std::make_shared<ParameterDef>();
            auto luaParam = luaParamPair.second.as<sol::table>();
            sol::object paramName = luaParam["name"];
            sol::object paramId = luaParam["id"];
            // TODO: default values with functions seems not to work properly
            // e.g. min and max needs to be set although it is impl. as optional
            param->name = paramName.as<ParameterDef::FStringGetter>();
            param->id = paramId.as<std::string>();
            param->type = require_key<std::string>(luaParam, "type");
            if (param->type == PARAM_TYPE_ENVELOPE) {
                param->setValue = optional_key<ParameterDef::FSetValue>(luaParam, "setValue", nullptr);
                sol::optional<sol::table> levelIdsTable = luaParam["levelIds"];
                if (levelIdsTable) {
                    for (auto& kv : *levelIdsTable)
                        param->levelIds.push_back(kv.second.as<std::string>());
                }
                sol::optional<sol::table> timeIdsTable = luaParam["timeIds"];
                if (timeIdsTable) {
                    for (auto& kv : *timeIdsTable)
                        param->timeIds.push_back(kv.second.as<std::string>());
                }
                param->sustainSegment = optional_key<bool>(luaParam, "sustainSegment", false);
            } else {
                param->setValue = require_key<ParameterDef::FSetValue>(luaParam, "setValue");
            }
            param->min = optional_key(luaParam, "min", param->min);
            param->max = optional_key(luaParam, "max", param->max);
            param->format = optional_key(luaParam, "format", param->format);
            param->value = optional_key(luaParam, "default", param->min ? param->min() : 0);
            param->toI7Value = optional_key<ParameterDef::FToI7Value>(luaParam, "toI7Value", nullptr);
            param->toGuiValue = optional_key<ParameterDef::FToGuiValue>(luaParam, "toGuiValue", nullptr);
            param->options = optional_key<ParameterDef::SelectionOptions>(luaParam, "options", ParameterDef::SelectionOptions());
            if (!param->options.empty())
            {
                param->stringValue = getDefaultValue(param->options, param->value);
            }
            outSectionDef.params.push_back(param.get());
            ed.parameterDefs[param->id] = param;
        }
    }
    if(lua_table["grp"] != sol::nil)
    {
        sol::table luaSubSections = lua_table["grp"];
        for(const auto& luaSubSectionObj : luaSubSections)
        {
            sol::table luaSubSection = luaSubSectionObj.second;
            SectionDef subSection;
            getSection(ed, luaSubSection, subSection);
            outSectionDef.subSections.push_back(subSection);
        }
    }
    if(lua_table["isOpen"] != sol::nil)
    {
        outSectionDef.isOpen = lua_table["isOpen"];
    }
}

void getDefs(I7Ed &ed, SectionDef::NamedSections &outNamedSections)
{
    sol::table mainSections = ed.lua["Main"];
    for(auto &luaSectionPair : mainSections)
    {
        sol::table luaSection = luaSectionPair.second.as<sol::table>();
        SectionDef sectionDef;
        getSection(ed, luaSection, sectionDef);
        outNamedSections.insert({luaSectionPair.first.as<std::string>(), sectionDef});
    }
}
// END: Lua to Def

void valueChanged(const sol::state &lua, RtMidiOut &midiOut, const ParameterDef& paramDef)
{
    try 
    {
        std::cout << paramDef.id << std::endl;
        int i7Value = (int)paramDef.value;
        if (paramDef.toI7Value)
        {
            i7Value = paramDef.toI7Value(paramDef.value);
        }
        auto sysex = paramDef.setValue(i7Value);
        sendMessage(midiOut, sysex);
    } catch(const sol::error error)
    {
        std::cerr << error.what() << std::endl;
    }
}

ParameterDef* getParameterDef(I7Ed &ed, const std::string &id); // forward declaration

void renderCombo(ParameterDef& param, I7Ed &ed)
{
    if (ImGui::BeginCombo(param.name().c_str(), param.stringValue.c_str()))
    {
        if (ImSearch::BeginSearch())
        {
            ImSearch::SearchBar();

            for (const auto& [value, label] : param.options)
            {
                ImSearch::SearchableItem(label.c_str(),
                    [&](const char* name)
                    {
                        const bool isSelected = value == param.value;
                        if (ImGui::Selectable(name, isSelected))
                        {
                            param.stringValue = name;
                            param.value = value;
                            valueChanged(ed.lua, ed.midiOut, param);
                        }
                    });
            }
            ImSearch::EndSearch();
        }
        ImGui::EndCombo();
    }
}

void renderSection(SectionDef &section, I7Ed &ed)
{
    constexpr float kRowSpacing = 15.0f;
    float lastKnobWidth = 0.0f;
    bool prevWasInline = false;
    bool isFirst = true;
    for(auto param : section.params)
    {
        if (param->name() == HIDDEN_PARAM_NAME)
        {
            continue;
        }
        bool isBlock = param->type == PARAM_TYPE_SELECTION
                    || param->type == PARAM_TYPE_TOGGLE
                    || param->type == PARAM_TYPE_ENVELOPE;
        bool doSameLine = false;
        if (prevWasInline && !isBlock)
        {
            float nextX = ImGui::GetItemRectMax().x + ImGui::GetStyle().ItemSpacing.x;
            float rightEdge = ImGui::GetWindowPos().x + ImGui::GetContentRegionMax().x;
            doSameLine = (nextX + lastKnobWidth) <= rightEdge;
        }
        if (doSameLine)
        {
            ImGui::SameLine();
        }
        else if (!isFirst)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + kRowSpacing);
        }
        isFirst = false;
        if (param->type == PARAM_TYPE_RANGE)
        {
            if (ImGuiKnobs::Knob(param->name().c_str(), &param->value, param->min(), param->max(), 0.0f, param->format.c_str(), ImGuiKnobVariant_Tick, 0 , ImGuiKnobFlags_AlwaysClamp))
            {
                valueChanged(ed.lua, ed.midiOut, *param);
            }
            lastKnobWidth = ImGui::GetItemRectSize().x;
            prevWasInline = true;
        }
        else if (param->type == PARAM_TYPE_SELECTION)
        {
            renderCombo(*param, ed);
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_TOGGLE)
        {
            auto oldValue = param->value;
            bool toggleVal = param->value != 0;
            if (ImGui::Toggle(param->name().c_str(), &toggleVal))
            {
                param->value = toggleVal ? 1.0f : 0.0f;
                valueChanged(ed.lua, ed.midiOut, *param);
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_ENVELOPE)
        {
            const int nLevels = (int)param->levelIds.size();
            const int nTimes  = (int)param->timeIds.size();
            std::vector<float*> lvlPtrs(nLevels, nullptr);
            std::vector<float*> timPtrs(nTimes,  nullptr);
            for (int i = 0; i < nLevels; ++i) {
                auto* p = getParameterDef(ed, param->levelIds[i]);
                if (p) lvlPtrs[i] = &p->value;
            }
            for (int i = 0; i < nTimes; ++i) {
                auto* p = getParameterDef(ed, param->timeIds[i]);
                if (p) timPtrs[i] = &p->value;
            }
            bool allValid = true;
            for (auto* lp : lvlPtrs) if (!lp) { allValid = false; break; }
            for (auto* tp : timPtrs) if (!tp) { allValid = false; break; }

            if (allValid) {
                auto* firstLevel = getParameterDef(ed, param->levelIds[0]);
                auto* firstTime  = getParameterDef(ed, param->timeIds[0]);
                const float levelMin = firstLevel->min();
                const float levelMax = firstLevel->max();
                const float timeMax  = firstTime->max();

                // snapshot values to detect changes
                std::vector<float> oldLvl(nLevels), oldTim(nTimes);
                for (int i = 0; i < nLevels; ++i) oldLvl[i] = *lvlPtrs[i];
                for (int i = 0; i < nTimes;  ++i) oldTim[i] = *timPtrs[i];

                if (ImEnvelope::EnvelopeWidget(param->id.c_str(),
                        lvlPtrs.data(), nLevels,
                        timPtrs.data(), nTimes,
                        levelMin, levelMax, timeMax,
                        ImVec2(0, 120.f), param->sustainSegment))
                {
                    for (int i = 0; i < nLevels; ++i) {
                        if (*lvlPtrs[i] != oldLvl[i]) {
                            auto* p = getParameterDef(ed, param->levelIds[i]);
                            if (p) valueChanged(ed.lua, ed.midiOut, *p);
                        }
                    }
                    for (int i = 0; i < nTimes; ++i) {
                        if (*timPtrs[i] != oldTim[i]) {
                            auto* p = getParameterDef(ed, param->timeIds[i]);
                            if (p) valueChanged(ed.lua, ed.midiOut, *p);
                        }
                    }
                }
            }
            prevWasInline = false;
        }
        else
        {
            std::cerr << "unknown param type: '" << param->type << "'" << std::endl;
            prevWasInline = false;
        }
    }
}

Bytes sendAndReceive(I7Ed &ed, const Bytes &bytes)
{
    sendMessage(ed.midiOut, bytes);
    Bytes answer;
    int idleMillis = 10;
    int timeOutSeconds = 5;
    int maxTries = (int(1000 / (double)idleMillis) * timeOutSeconds);
    while (maxTries-- > 0) {
        ed.midiIn.getMessage(&answer);
        if (!answer.empty()) 
        {
            return answer;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(idleMillis));
    }
    std::cerr << "MIDI receive timed out." << std::endl;
    return answer;
}

ParameterDef* getParameterDef(I7Ed &ed, const std::string &id)
{
    auto mapIt = ed.parameterDefs.find(id);
    if (mapIt == ed.parameterDefs.end())
    {
        return nullptr;
    }
    return mapIt->second.get();
}

void valueChanged(I7Ed &ed, const ValueChangedMessage& vcMessage)
{
    auto paramDef = getParameterDef(ed, vcMessage.id);
    if (paramDef == nullptr)
    {
        std::cerr << "unknown parameter change received: " << vcMessage.id << std::endl;
        return;
    }
    float guiValue = vcMessage.i7Value;
    if (paramDef->toGuiValue)
    {
        guiValue = paramDef->toGuiValue(vcMessage.i7Value);
    }
    paramDef->value = guiValue;
    if (!paramDef->options.empty())
    {
        paramDef->stringValue = paramDef->options.at((int)paramDef->value);
    }
}

void performValuesReceive(I7Ed &ed, const SectionDef &section)
{
    auto requests = section.getReceiveSysex();
    for(const auto& request : requests)
    {
        auto received = sendAndReceive(ed, request.sysex);
        if (received.empty())
        {
            break;
        }
        auto valueChangeMsgs = request.onMessageReceived(received);
        for(const auto &valueChangeMsg : valueChangeMsgs)
        {
            if (valueChangeMsg.id.empty())
            {
                continue;
            }
            valueChanged(ed, valueChangeMsg);
        }
    }
}

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
    if (ed.args.listInputs)
    {
        std::cout << "Inputs:"  << std::endl;
        printMidiIo<RtMidiIn>();
    }
    if (ed.args.listOutputs)
    {
        std::cout << "Outputs:"  << std::endl;
        printMidiIo<RtMidiOut>();
    }
    if (ed.args.listOutputs || ed.args.listInputs)
    {
        return 0;
    }

    if (ed.args.inPortNr >= 0)
    {
        openPort(ed.midiIn, (size_t)ed.args.inPortNr);
        ed.midiIn.ignoreTypes(false, false, false);
    }
    if (ed.args.outPortNr >= 0)
    {
        openPort(ed.midiOut, (size_t)ed.args.outPortNr);
    }
    
    const char *luaFile = ed.args.mainLuaFilePath.empty() 
        ?  "./lua/main.lua" 
        : ed.args.mainLuaFilePath.c_str();
    
    ed.lua.new_usertype<RequestMessage>("RequestMessage", 
        "sysex", &RequestMessage::sysex,
        "onMessageReceived", &RequestMessage::onMessageReceived
    );
    ed.lua.new_usertype<ValueChangedMessage>("ValueChangedMessage", 
        "id", &ValueChangedMessage::id,
        "i7Value", &ValueChangedMessage::i7Value
    );

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
    glfwSwapInterval(1); // V-Sync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImCmd::CreateContext();
    ImSearch::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); 

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	ed.lua.open_libraries();
    ed.lua.script_file(luaFile);

    SectionDef::NamedSections sections;
    getDefs(ed, sections);
    bool show_command_palette = false;

    for(auto &section : sections)
    {
        // open section
        ImCmd::Command cmd;
        cmd.Name = std::string("open ") + section.second.name;
        cmd.InitialCallback = [&section]() {
            section.second.isOpen = true;
        };
        ImCmd::AddCommand(std::move(cmd));
        // receive
        if (!section.second.getReceiveSysex)
        {
            continue;
        }
        cmd.Name = std::string("receive ") + section.second.name;
        cmd.InitialCallback = [&ed, &section]() {
            if (ed.isReceiving.exchange(true)) return; // already in progress
            if (ed.receiveThread.joinable()) ed.receiveThread.join();
            // getReceiveSysex calls Lua — must happen on main thread before the thread starts
            auto requests = section.second.getReceiveSysex();
            ed.receiveStartTime = std::chrono::steady_clock::now();
            ed.receiveThread = std::thread([&ed, requests = std::move(requests)]() {
                for (const auto& req : requests) {
                    Bytes received = sendAndReceive(ed, req.sysex);
                    if (received.empty()) {
                        ed.notifications.push("MIDI receive timed out");
                        break;
                    }
                    std::lock_guard<std::mutex> lock(ed.pendingMutex);
                    ed.pendingReceives.push_back({req.onMessageReceived, std::move(received)});
                }
                ed.isReceiving.store(false);
            });
        };
        ImCmd::AddCommand(std::move(cmd));
    }

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Apply MIDI receive results on the main thread (Lua calls safe here)
        {
            std::vector<PendingReceive> toProcess;
            {
                std::lock_guard<std::mutex> lock(ed.pendingMutex);
                toProcess.swap(ed.pendingReceives);
            }
            for (auto& item : toProcess) {
                auto msgs = item.handler(item.data);
                for (const auto& msg : msgs) {
                    if (!msg.id.empty()) valueChanged(ed, msg);
                }
            }
        }

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // YouTube-style red progress bar at the top while receiving
        if (ed.isReceiving.load()) {
            auto elapsed = std::chrono::duration<float>(
                std::chrono::steady_clock::now() - ed.receiveStartTime).count();
            const float progress = std::min(1.f - std::exp(-elapsed * 0.7f), 0.9f);
            ImGui::GetBackgroundDrawList()->AddRectFilled(
                ImVec2(0.f, 0.f),
                ImVec2(progress * (float)display_w, 3.f),
                IM_COL32(220, 30, 30, 255));
        }

        ed.notifications.render(display_w, display_h);

        if (io.KeyCtrl && io.KeyShift && ImGui::IsKeyPressed(ImGuiKey_P)) {
            show_command_palette = !show_command_palette;
        }
        if (show_command_palette) {
            ImCmd::CommandPaletteWindow("CommandPalette", &show_command_palette);
        }

        for(auto &sectionPair : sections)
        {
            auto& section = sectionPair.second;
            if (!section.isOpen)
            {
                continue;
            }
            if (ImGui::Begin(section.name.c_str(), &section.isOpen))
            {
                renderSection(section, ed);
                for(auto &subSection : section.subSections)
                {
                    renderSection(subSection, ed);
                }
            }
            ImGui::End();
        }
        ImGui::Render();
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    if (ed.receiveThread.joinable()) ed.receiveThread.join();

    ImCmd::DestroyContext();
    ImSearch::DestroyContext();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    if (ed.midiIn.isPortOpen())
    {
         ed.midiIn.closePort();
    }
    if (ed.midiOut.isPortOpen())
    {
        ed.midiOut.closePort();
    }
    return 0;
}