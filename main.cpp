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
#include <string>
#include "ParameterDef.h"
#include "imgui_envelope.h"
#include "imgui_midi_leds.h"
#include "imgui_step_lfo.h"
#include "imgui_notifications.h"
#include "imgui_internal.h"
#include <vector>
#include <list> // TODO: use queue
#include <tuple>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <set>
#include <unordered_map>
#include <memory>
#include "Com.h"
#include "Midi.h"
#include "ImGuiFileDialog.h"
#include <fstream>

#define HIDDEN_PARAM_NAME "__HIDDEN__"

namespace
{
    const float HighlightSeconds = 15.0f;
}

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

enum class ToneType
{
    Unknown = 0,
    SNA  = 1,
    SNS  = 2,
    SND  = 3,
    PCMS = 4,
    PCMD = 5,
};

struct SidebarState
{
    static constexpr float kWidth = 250.0f;
};

struct I7Ed
{
    Args args;
    Midi midi;
    sol::state lua;
    std::unordered_map<std::string, std::shared_ptr<ParameterDef>> parameterDefs;
    // async receive
    std::atomic<bool> isReceiving{false};
    std::chrono::steady_clock::time_point receiveStartTime;
    std::list<PendingReceive> pendingReceives;
    std::mutex pendingMutex;
    NotificationQueue notifications;
    std::atomic<int64_t> midiSendTimeNs{0};
    std::atomic<int64_t> midiRecvTimeNs{0};
    // param search highlight
    std::string highlightParamId;
    float       highlightTimer = 0.f;
    // Save SysEx
    SectionDef::NamedSections* pSections = nullptr;
    struct LoadSysexState
    {
        std::string filepath;
    } loadSysex;
    struct SaveSysexState
    {
        enum class Phase { Idle, ReadMsb, ReadTone };
        Phase       phase = Phase::Idle;
        std::string filepath;
        std::string partPrefix;              // "Part 01 "
        std::vector<std::string> tonePrefixes; // set after MSB is received
    } saveSysex;
    SidebarState sidebar;
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


// BEGIN: LUA to DEF. TODO Refactor into separate file
template<typename T>
T require_key(sol::table tbl, const std::string& key)
{
    sol::optional<T> val = tbl[key];
    if (!val)
    {
        throw std::runtime_error("Lua table is missing required key: " + key);
    }
    return val.value();
}

template<typename T>
T optional_key(sol::table tbl, const std::string& key, const T& defaultValue)
{
    sol::optional<T> val = tbl[key];
    if (!val.has_value())
    {
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
            if (param->type == PARAM_TYPE_ENVELOPE)
            {
                param->setValue = optional_key<ParameterDef::FSetValue>(luaParam, "setValue", nullptr);
                sol::optional<sol::table> levelIdsTable = luaParam["levelIds"];
                if (levelIdsTable)
                {
                    for (auto& kv : *levelIdsTable)
                    {
                        param->levelIds.push_back(kv.second.as<std::string>());
                    }
                }
                sol::optional<sol::table> timeIdsTable = luaParam["timeIds"];
                if (timeIdsTable)
                {
                    for (auto& kv : *timeIdsTable)
                    {
                        param->timeIds.push_back(kv.second.as<std::string>());
                    }
                }
                param->sustainSegment = optional_key<bool>(luaParam, "sustainSegment", false);
            }
            else if (param->type == PARAM_TYPE_STEP_LFO)
            {
                param->setValue = optional_key<ParameterDef::FSetValue>(luaParam, "setValue", nullptr);
                param->stepTypeId = optional_key<std::string>(luaParam, "stepTypeId", "");
                sol::optional<sol::table> stepIdsTable = luaParam["stepIds"];
                if (stepIdsTable)
                {
                    for (auto& kv : *stepIdsTable)
                    {
                        param->stepIds.push_back(kv.second.as<std::string>());
                    }
                }
            }
            else if (param->type == PARAM_TYPE_ACTION)
            {
                param->getAction = require_key<ParameterDef::FGetAction>(luaParam, "getAction");
            }
            else if (param->type == PARAM_TYPE_SAVE_SYSEX
                  || param->type == PARAM_TYPE_LOAD_SYSEX)
            {
                param->partPrefix = optional_key<std::string>(luaParam, "partPrefix", "");
            }
            else
            {
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
    if(lua_table["hideFromPalette"] != sol::nil)
    {
        outSectionDef.hideFromPalette = lua_table["hideFromPalette"];
    }
    if(lua_table["tabs"] != sol::nil)
    {
        if(lua_table["tabCommonKey"] != sol::nil)
        {
            outSectionDef.tabCommonKey = lua_table["tabCommonKey"].get<std::string>();
        }
        sol::table luaTabs = lua_table["tabs"];
        for(const auto& tabPair : luaTabs)
        {
            sol::table luaTab = tabPair.second.as<sol::table>();
            SectionDef::TabEntry entry;
            entry.label = luaTab["label"].get<std::string>();
            sol::table keys = luaTab["keys"];
            for(const auto& kp : keys)
            {
                entry.sectionKeys.push_back(kp.second.as<std::string>());
            }
            outSectionDef.tabs.push_back(entry);
        }
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

void sendMessage(I7Ed &ed, const Bytes& message)
{
    if (message.empty())
    {
        return;
    }
    ed.midi.sendMessage(message);
}

void valueChanged(I7Ed &ed, const ParameterDef& paramDef)
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
        sendMessage(ed, sysex);
    } catch(const sol::error error)
    {
        std::cerr << error.what() << std::endl;
    }
}

ParameterDef* getParameterDef(I7Ed &ed, const std::string &id); // forward declaration
void valueChanged(I7Ed &ed, const ValueChangedMessage& vcMessage); // forward declaration

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
                            valueChanged(ed, param);
                        }
                    });
            }
            ImSearch::EndSearch();
        }
        ImGui::EndCombo();
    }
}

void renderSection(SectionDef &section, I7Ed &ed);

void saveSysexToFile(I7Ed &ed)
{
    if (!ed.pSections || ed.saveSysex.filepath.empty())
    {
        return;
    }
    const std::string& prefix = ed.saveSysex.partPrefix;
    std::ofstream file(ed.saveSysex.filepath, std::ios::binary);
    if (!file)
    {
        ed.notifications.push("Error: could not open file: " + ed.saveSysex.filepath,
                              ImVec4(1.f, 0.2f, 0.2f, 1.f));
        return;
    }
    int count = 0;
    // Helper: write sysex for one param by id
    auto writeParam = [&](const std::string& paramId)
    {
        auto it = ed.parameterDefs.find(paramId);
        if (it == ed.parameterDefs.end())
        {
            return;
        }
        auto* p = it->second.get();
        if (!p->setValue)
        {
            return;
        }
        try
        {
            int i7v = p->toI7Value ? p->toI7Value(p->value) : (int)p->value;
            Bytes sysex = p->setValue(i7v);
            if (!sysex.empty() && sysex[0] == 0xF0)
            {
                file.write(reinterpret_cast<const char*>(sysex.data()),
                           static_cast<std::streamsize>(sysex.size()));
                ++count;
            }
        }
        catch (const sol::error& e)
        {
            std::cerr << "saveSysex: " << paramId << ": " << e.what() << std::endl;
        }
    };
    // Helper: write all saveable params of one section
    auto writeSection = [&](const SectionDef& sec)
    {
        for (const auto* param : sec.params)
        {
            if (!param)
            {
                continue;
            }
            if (param->type == PARAM_TYPE_RANGE
             || param->type == PARAM_TYPE_SELECTION
             || param->type == PARAM_TYPE_TOGGLE)
            {
                writeParam(param->id);
            }
            else if (param->type == PARAM_TYPE_ENVELOPE)
            {
                for (const auto& id : param->levelIds) { writeParam(id); }
                for (const auto& id : param->timeIds)  { writeParam(id); }
            }
            else if (param->type == PARAM_TYPE_STEP_LFO)
            {
                if (!param->stepTypeId.empty()) { writeParam(param->stepTypeId); }
                for (const auto& id : param->stepIds) { writeParam(id); }
            }
        }
    };
    auto keyMatches = [&](const std::string& key) -> bool
    {
        for (const auto& pfx : ed.saveSysex.tonePrefixes)
        {
            if (key.size() >= pfx.size() && key.substr(0, pfx.size()) == pfx)
            {
                return true;
            }
        }
        return false;
    };
    for (auto& [key, sec] : *ed.pSections)
    {
        if (!keyMatches(key))
        {
            continue;
        }
        writeSection(sec);
        for (const auto& sub : sec.subSections)
        {
            writeSection(sub);
        }
    }
    file.close();
    std::string msg = "Saved " + std::to_string(count) + " SysEx to " + ed.saveSysex.filepath;
    ed.notifications.push(msg, ImVec4(0.2f, 1.f, 0.4f, 1.f), 5.f, 3.5f);
}

// Builds one RequestMessage per param that is registered in parameterDefs.
// Covers range/select/toggle directly and levelIds/timeIds/stepIds for
// envelope and step-lfo meta-params.
std::vector<RequestMessage> buildParamRequests(
    I7Ed& ed,
    const std::vector<ParameterDef*>& params)
{
    sol::function fn = ed.lua["CreateReceiveMessageForLeafId"];
    std::vector<RequestMessage> result;
    auto tryAdd = [&](const std::string& id)
    {
        if (!ed.parameterDefs.count(id))
        {
            return;
        }
        sol::object obj = fn(id);
        if (!obj.valid() || obj.get_type() == sol::type::nil)
        {
            return;
        }
        result.push_back(obj.as<RequestMessage>());
    };
    for (const auto* param : params)
    {
        if (!param)
        {
            continue;
        }
        if (param->type == PARAM_TYPE_RANGE
         || param->type == PARAM_TYPE_SELECTION
         || param->type == PARAM_TYPE_TOGGLE)
        {
            tryAdd(param->id);
        }
        else if (param->type == PARAM_TYPE_ENVELOPE)
        {
            for (const auto& id : param->levelIds) { tryAdd(id); }
            for (const auto& id : param->timeIds)  { tryAdd(id); }
        }
        else if (param->type == PARAM_TYPE_STEP_LFO)
        {
            if (!param->stepTypeId.empty()) { tryAdd(param->stepTypeId); }
            for (const auto& id : param->stepIds) { tryAdd(id); }
        }
    }
    return result;
}

// Returns a FGetReceiveSysex getter that only requests params known to parameterDefs.
SectionDef::FGetReceiveSysex makeParamOnlyGetter(I7Ed& ed, const SectionDef& sec)
{
    std::vector<ParameterDef*> params(sec.params.begin(), sec.params.end());
    for (const auto& sub : sec.subSections)
    {
        for (auto* p : sub.params) { params.push_back(p); }
    }
    return [&ed, params]() -> std::vector<RequestMessage>
    {
        return buildParamRequests(ed, params);
    };
}

void loadSysexFromFile(I7Ed& ed)
{
    if (ed.loadSysex.filepath.empty())
    {
        return;
    }
    // Build addr -> onMessageReceived map from ALL params in parameterDefs.
    // Reuses buildParamRequests; the RQ1 sysex encodes the address at bytes 7-10.
    std::vector<ParameterDef*> allParams;
    allParams.reserve(ed.parameterDefs.size());
    for (auto& [id, param] : ed.parameterDefs)
    {
        allParams.push_back(param.get());
    }
    auto allRequests = buildParamRequests(ed, allParams);

    std::unordered_map<uint32_t, RequestMessage::FOnMessageReceived> addrMap;
    addrMap.reserve(allRequests.size());
    for (const auto& req : allRequests)
    {
        if (req.sysex.size() >= 11)
        {
            uint32_t addr = ((uint32_t)req.sysex[7] << 24)
                          | ((uint32_t)req.sysex[8] << 16)
                          | ((uint32_t)req.sysex[9]  << 8)
                          |  (uint32_t)req.sysex[10];
            addrMap[addr] = req.onMessageReceived;
        }
    }

    std::ifstream file(ed.loadSysex.filepath, std::ios::binary);
    if (!file)
    {
        ed.notifications.push("Error: could not open file: " + ed.loadSysex.filepath,
                              ImVec4(1.f, 0.2f, 0.2f, 1.f));
        return;
    }
    Bytes fileBytes((std::istreambuf_iterator<char>(file)), {});
    file.close();

    int count = 0;
    for (size_t i = 0; i < fileBytes.size(); )
    {
        if (fileBytes[i] != 0xF0)
        {
            ++i;
            continue;
        }
        auto endIt = std::find(fileBytes.begin() + (ptrdiff_t)i, fileBytes.end(),
                               (unsigned char)0xF7);
        if (endIt == fileBytes.end())
        {
            break;
        }
        ++endIt;
        Bytes msg(fileBytes.begin() + (ptrdiff_t)i, endIt);

        // DT1: byte[6] == 0x12, address at bytes 7-10
        if (msg.size() >= 12 && msg[6] == 0x12)
        {
            uint32_t addr = ((uint32_t)msg[7] << 24)
                          | ((uint32_t)msg[8] << 16)
                          | ((uint32_t)msg[9]  << 8)
                          |  (uint32_t)msg[10];
            auto it = addrMap.find(addr);
            if (it != addrMap.end())
            {
                auto vcMsgs = it->second(msg);
                for (const auto& vcm : vcMsgs)
                {
                    if (!vcm.id.empty())
                    {
                        valueChanged(ed, vcm);        // update GUI
                        auto* p = getParameterDef(ed, vcm.id);
                        if (p && p->setValue)
                        {
                            valueChanged(ed, *p);     // send to device
                        }
                        ++count;
                    }
                }
            }
        }
        i = (size_t)(endIt - fileBytes.begin());
    }
    std::string msg = "Loaded " + std::to_string(count) + " params from " + ed.loadSysex.filepath;
    ed.notifications.push(msg, ImVec4(0.2f, 1.f, 0.4f, 1.f), 5.f, 3.5f);
}

std::vector<std::string> getTonePrefixes(const std::string& partPrefix, int msb)
{
    switch (msb)
    {
        case 89: return {partPrefix + "SNA", partPrefix + "MFX"};
        case 95: return {partPrefix + "SN-S "};
        case 88: return {partPrefix + "SN-D "};
        case 87: return {partPrefix + "PCM-S "};
        case 86: return {partPrefix + "PCM-D "};
        default: return {};
    }
}

void triggerReceive(I7Ed &ed, const std::vector<SectionDef::FGetReceiveSysex> &getters)
{
    if (ed.isReceiving.exchange(true))
    {
        return;
    }
    // Collect all requests on main thread (Lua calls must happen here)

    ed.receiveStartTime = std::chrono::steady_clock::now();
    
    bool break_ = false;
    for (const auto &getter : getters)
    {
        if (!getter)
        {
            continue;
        }
        std::vector<RequestMessage> reqs = getter();
        for(const RequestMessage &req : reqs)
        {
            if (break_)
            {
                break;
            }
            ed.pendingReceives.push_back({
                .handler = req.onMessageReceived
            });
            ed.midi.sendAndReceive(req.sysex, &ed.pendingReceives.back(), [&ed, &req, &break_](Bytes received, void* userData)
            {
                if (received.empty())
                {
                    break_ = true;
                    return;
                }
                std::lock_guard<std::mutex> lock(ed.pendingMutex);
                if (userData == nullptr)
                {
                    return;
                }
                PendingReceive *pData = (PendingReceive*)userData;
                pData->data.swap(received);
            });
        }
    }
}


static void drawReceiveButton(I7Ed &ed, const std::vector<SectionDef::FGetReceiveSysex> &getters)
{
    float btnW = ImGui::CalcTextSize("recv").x + ImGui::GetStyle().FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - btnW);
    if (ImGui::SmallButton("recv"))
    {
        triggerReceive(ed, getters);
    }
}

void renderTabbedSection(SectionDef &section, SectionDef::NamedSections &sections, I7Ed &ed, ImVec2 &canvasMax)
{
    if (!ImGui::Begin(section.name.c_str(), &section.isOpen))
    {
        ImGui::End();
        return;
    }
    {
        ImVec2 wp = ImGui::GetWindowPos(), ws = ImGui::GetWindowSize();
        canvasMax.x = std::max(canvasMax.x, wp.x + ws.x);
        canvasMax.y = std::max(canvasMax.y, wp.y + ws.y);
    }
    // Collect receive getters from all referenced sections
    std::vector<SectionDef::FGetReceiveSysex> getters;
    auto collectGetter = [&](const std::string &key)
    {
        auto it = sections.find(key);
        if (it != sections.end() && it->second.getReceiveSysex)
        {
            getters.push_back(it->second.getReceiveSysex);
        }
    };
    if (!section.tabCommonKey.empty())
    {
        collectGetter(section.tabCommonKey);
    }
    for (const auto &tab : section.tabs)
    {
        for (const auto &key : tab.sectionKeys)
        {
            collectGetter(key);
        }
    }

    if (!getters.empty())
    {
        drawReceiveButton(ed, getters);
    }

    // Optional common section rendered above the tab bar
    if (!section.tabCommonKey.empty())
    {
        auto it = sections.find(section.tabCommonKey);
        if (it != sections.end())
        {
            renderSection(it->second, ed);
            ImGui::Separator();
        }
    }
    if (ImGui::BeginTabBar("##tabs"))
    {
        for (int ti = 0; ti < (int)section.tabs.size(); ++ti)
        {
            const auto &tab = section.tabs[ti];
            if (ImGui::BeginTabItem(tab.label.c_str()))
            {
                ImGui::PushID(ti);
                for (const auto &key : tab.sectionKeys)
                {
                    auto it = sections.find(key);
                    if (it != sections.end())
                    {
                        renderSection(it->second, ed);
                        for (auto &sub : it->second.subSections)
                        {
                            renderSection(sub, ed);
                        }
                    }
                }
                ImGui::PopID();
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
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
                    || param->type == PARAM_TYPE_ENVELOPE
                    || param->type == PARAM_TYPE_STEP_LFO;
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
                valueChanged(ed, *param);
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
                valueChanged(ed, *param);
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_ENVELOPE)
        {
            const int nLevels = (int)param->levelIds.size();
            const int nTimes  = (int)param->timeIds.size();
            std::vector<float*> lvlPtrs(nLevels, nullptr);
            std::vector<float*> timPtrs(nTimes,  nullptr);
            for (int i = 0; i < nLevels; ++i)
            {
                auto* p = getParameterDef(ed, param->levelIds[i]);
                if (p)
                {
                    lvlPtrs[i] = &p->value;
                }
            }
            for (int i = 0; i < nTimes; ++i)
            {
                auto* p = getParameterDef(ed, param->timeIds[i]);
                if (p)
                {
                    timPtrs[i] = &p->value;
                }
            }
            bool allValid = true;
            for (auto* lp : lvlPtrs)
            {
                if (!lp)
                {
                    allValid = false;
                    break;
                }
            }
            for (auto* tp : timPtrs)
            {
                if (!tp)
                {
                    allValid = false;
                    break;
                }
            }

            if (allValid)
            {
                auto* firstLevel = getParameterDef(ed, param->levelIds[0]);
                auto* firstTime  = getParameterDef(ed, param->timeIds[0]);
                const float levelMin = firstLevel->min();
                const float levelMax = firstLevel->max();
                const float timeMax  = firstTime->max();

                // snapshot values to detect changes
                std::vector<float> oldLvl(nLevels), oldTim(nTimes);
                for (int i = 0; i < nLevels; ++i)
                {
                    oldLvl[i] = *lvlPtrs[i];
                }
                for (int i = 0; i < nTimes; ++i)
                {
                    oldTim[i] = *timPtrs[i];
                }

                if (ImEnvelope::EnvelopeWidget(param->id.c_str(),
                        lvlPtrs.data(), nLevels,
                        timPtrs.data(), nTimes,
                        levelMin, levelMax, timeMax,
                        ImVec2(0, 120.f), param->sustainSegment))
                {
                    for (int i = 0; i < nLevels; ++i)
                    {
                        if (*lvlPtrs[i] != oldLvl[i])
                        {
                            auto* p = getParameterDef(ed, param->levelIds[i]);
                            if (p)
                            {
                                valueChanged(ed, *p);
                            }
                        }
                    }
                    for (int i = 0; i < nTimes; ++i)
                    {
                        if (*timPtrs[i] != oldTim[i])
                        {
                            auto* p = getParameterDef(ed, param->timeIds[i]);
                            if (p)
                            {
                                valueChanged(ed, *p);
                            }
                        }
                    }
                }
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_STEP_LFO)
        {
            const int nSteps = (int)param->stepIds.size();
            std::vector<float*> stepPtrs(nSteps, nullptr);
            for (int i = 0; i < nSteps; ++i)
            {
                auto* p = getParameterDef(ed, param->stepIds[i]);
                if (p)
                {
                    stepPtrs[i] = &p->value;
                }
            }
            bool allValid = true;
            for (auto* sp : stepPtrs)
            {
                if (!sp)
                {
                    allValid = false;
                    break;
                }
            }

            if (allValid)
            {
                auto* firstStep = getParameterDef(ed, param->stepIds[0]);
                const float valMin = firstStep->min();
                const float valMax = firstStep->max();

                float stepType = 0.f;
                auto* typeParam = getParameterDef(ed, param->stepTypeId);
                if (typeParam)
                {
                    stepType = typeParam->value;
                }

                std::vector<float> oldVals(nSteps);
                for (int i = 0; i < nSteps; ++i)
                {
                    oldVals[i] = *stepPtrs[i];
                }

                if (ImStepLfo::StepLfoWidget(param->id.c_str(),
                        stepPtrs.data(), nSteps, stepType,
                        valMin, valMax, ImVec2(0, 80.f)))
                {
                    for (int i = 0; i < nSteps; ++i)
                    {
                        if (*stepPtrs[i] != oldVals[i])
                        {
                            auto* p = getParameterDef(ed, param->stepIds[i]);
                            if (p)
                            {
                                valueChanged(ed, *p);
                            }
                        }
                    }
                }
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_ACTION)
        {
            if (ImGui::Button(param->name().c_str()))
            {
                triggerReceive(ed, {param->getAction});
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_SAVE_SYSEX)
        {
            if (ImGui::Button(param->name().c_str()))
            {
                ed.saveSysex.partPrefix = param->partPrefix;
                IGFD::FileDialogConfig cfg;
                cfg.path      = ".";
                cfg.fileName  = "patch.syx";
                cfg.countSelectionMax = 1;
                cfg.flags     = ImGuiFileDialogFlags_ConfirmOverwrite;
                ImGuiFileDialog::Instance()->OpenDialog(
                    "SaveSysexDlg", "Save SysEx File", ".syx", cfg);
            }
            prevWasInline = false;
        }
        else if (param->type == PARAM_TYPE_LOAD_SYSEX)
        {
            if (ImGui::Button(param->name().c_str()))
            {
                IGFD::FileDialogConfig cfg;
                cfg.path      = ".";
                cfg.countSelectionMax = 1;
                cfg.flags     = 0;
                ImGuiFileDialog::Instance()->OpenDialog(
                    "LoadSysexDlg", "Load SysEx File", ".syx", cfg);
            }
            prevWasInline = false;
        }
        else
        {
            std::cerr << "unknown param type: '" << param->type << "'" << std::endl;
            prevWasInline = false;
        }

        // Yellow highlight for the parameter found via "?" search
        if (!ed.highlightParamId.empty()
            && param->id == ed.highlightParamId
            && ed.highlightTimer > 0.f)
        {
            const float alpha = std::min(1.f, ed.highlightTimer);
            constexpr float pad = 4.f;
            const ImVec2 r0 = ImGui::GetItemRectMin();
            const ImVec2 r1 = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRect(
                {r0.x - pad, r0.y - pad}, {r1.x + pad, r1.y + pad},
                IM_COL32(255, 220, 0, (int)(alpha * 220)), 4.f, 0, 2.f);
        }
    }
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

static ToneType getPartToneType(I7Ed& ed, int partNr)
{
    std::string id = "PRM-_PRF-_FP" + std::to_string(partNr) + "-NEFP_TYPE_DUMMY";
    ParameterDef* p = getParameterDef(ed, id);
    if (p == nullptr)
    {
        return ToneType::Unknown;
    }
    int v = (int)p->value;
    if (v < 1 || v > 5)
    {
        return ToneType::Unknown;
    }
    return static_cast<ToneType>(v);
}

static void renderPartButtons(SectionDef::NamedSections& sections, int partNr, ToneType type)
{
    char pn[4];
    snprintf(pn, sizeof(pn), "%02d", partNr);
    std::string base = std::string("Part ") + pn + " ";

    auto viewButton = [&](const char* label, const std::string& key) -> void
    {
        auto it = sections.find(key);
        if (it == sections.end())
        {
            return;
        }
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

    switch (type)
    {
        case ToneType::SNA:
        {
            viewButton("SNA", base + "SNA");
            viewButton("MFX", base + "MFX");
            break;
        }
        case ToneType::SNS:
        {
            std::string pfx = base + "SN-S ";
            viewButton("OSC",    pfx + "OSC");
            viewButton("Pitch",  pfx + "Pitch");
            viewButton("Filter", pfx + "Filter");
            viewButton("Amp",    pfx + "Amp");
            viewButton("LFO",    pfx + "LFO");
            viewButton("MFX",    pfx + "MFX");
            break;
        }
        case ToneType::SND:
        {
            std::string pfx = base + "SN-D ";
            viewButton("Common", pfx + "Common");
            viewButton("CompEq", pfx + "CompEq");
            viewButton("MFX",    pfx + "MFX");
            break;
        }
        case ToneType::PCMS:
        {
            std::string pfx = base + "PCM-S ";
            viewButton("Common",  pfx + "Common");
            viewButton("Wave",    pfx + "Wave");
            viewButton("PMT",     pfx + "PMT");
            viewButton("Pitch",   pfx + "Pitch All");
            viewButton("TVF",     pfx + "TVF");
            viewButton("TVA",     pfx + "TVA");
            viewButton("LFO1",    pfx + "LFO1");
            viewButton("LFO2",    pfx + "LFO2");
            viewButton("StepLFO", pfx + "Step LFO");
            viewButton("CTRL",    pfx + "CTRL");
            viewButton("MTRX",    pfx + "MTRX CTRL");
            viewButton("Output",  pfx + "Output");
            viewButton("MFX",     pfx + "MFX");
            break;
        }
        case ToneType::PCMD:
        {
            std::string pfx = base + "PCM-D ";
            viewButton("Common", pfx + "Common");
            viewButton("Pitch",  pfx + "Pitch");
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

static void renderSidebar(I7Ed& ed, SectionDef::NamedSections& sections)
{
    for (int partIdx = 0; partIdx < 16; ++partIdx)
    {
        int partNr = partIdx + 1;
        char pn[4];
        snprintf(pn, sizeof(pn), "%02d", partNr);
        std::string headerLabel = std::string("Part ") + pn;

        ImGui::PushID(partIdx);
        if (ImGui::CollapsingHeader(headerLabel.c_str()))
        {
            std::string typeParamId = "PRM-_PRF-_FP"
                + std::to_string(partNr)
                + "-NEFP_TYPE_DUMMY";
            ParameterDef* typeParam = getParameterDef(ed, typeParamId);
            if (typeParam != nullptr)
            {
                ImGui::SetNextItemWidth(-1.0f);
                renderCombo(*typeParam, ed);
            }
            ToneType type = getPartToneType(ed, partNr);
            renderPartButtons(sections, partNr, type);
        }
        ImGui::PopID();
    }
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
        ed.midi.printInputs(std::cout);
    }
    if (ed.args.listOutputs)
    {
        std::cout << "Outputs:"  << std::endl;
        ed.midi.printOutputs(std::cout);
    }
    if (ed.args.listOutputs || ed.args.listInputs)
    {
        return 0;
    }

    if (ed.args.inPortNr >= 0)
    {
        ed.midi.openInput((size_t)ed.args.inPortNr);
    }
    if (ed.args.outPortNr >= 0)
    {
         ed.midi.openOutput((size_t)ed.args.inPortNr);
    }

    ed.midi.start();
    ed.midi.onReceive = [&ed]()
    {
        ed.midiRecvTimeNs.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order_relaxed);
    };
    ed.midi.onSend = [&ed]()
    {
        ed.midiSendTimeNs.store(std::chrono::steady_clock::now().time_since_epoch().count(), std::memory_order_relaxed);
    };

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
    glfwSwapInterval(0); // no blocking swap - idle throttling done manually below

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImCmd::CreateContext();
    ImSearch::CreateContext();

    ImGuiIO& io = ImGui::GetIO();

    // Persist which sections are open across sessions, stored in imgui.ini
    // under the custom [I7EdOpenSections] block.
    struct OpenSectionsData
    {
        std::vector<std::string> pending;   // section keys read from ini
        SectionDef::NamedSections* pSections = nullptr;
    };
    SectionDef::NamedSections sections;
    OpenSectionsData openData;
    openData.pSections = &sections;

    {
        ImGuiSettingsHandler h = {};
        h.TypeName    = "I7EdOpenSections";
        h.TypeHash    = ImHashStr("I7EdOpenSections");
        h.UserData    = &openData;
        h.ReadOpenFn  = [](ImGuiContext*, ImGuiSettingsHandler*, const char*) -> void*
        {
            return (void*)1;    // non-null signals "recognised section, read it"
        };
        h.ReadLineFn  = [](ImGuiContext*, ImGuiSettingsHandler* handler, void*, const char* line)
        {
            auto* d = static_cast<OpenSectionsData*>(handler->UserData);
            if (line[0] != '\0')
            {
                d->pending.push_back(line);
            }
        };
        h.WriteAllFn  = [](ImGuiContext*, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
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

    // Explicitly load ini now so pending open-sections are known before getDefs.
    if (io.IniFilename)
    {
        ImGui::LoadIniSettingsFromDisk(io.IniFilename);
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

	ed.lua.open_libraries();
    ed.lua.script_file(luaFile);

    getDefs(ed, sections);
    ed.pSections = &sections;

    // Restore sections that were open in the previous session.
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

    bool show_command_palette = false;

    for(auto &section : sections)
    {
        if (section.second.hideFromPalette)
        {
            continue;
        }
        // open section
        ImCmd::Command cmd;
        cmd.Name = std::string("open ") + section.second.name;
        cmd.InitialCallback = [&section]()
        {
            section.second.isOpen = true;
        };
        ImCmd::AddCommand(std::move(cmd));
    }

    // Parameter search: "? ParamName (Section)" commands
    // Typing "?" in the command palette filters to these and opens the containing view.
    {
        // Build reverse map: hidden section key → pointer to the tabbed parent that owns it.
        // std::map references are stable so raw pointers are safe here.
        std::unordered_map<std::string, SectionDef *> hiddenToParent;
        for(auto &[tKey, tSec] : sections)
        {
            if (tSec.tabs.empty())
            {
                continue;
            }
            if (!tSec.tabCommonKey.empty())
            {
                hiddenToParent[tSec.tabCommonKey] = &sections.at(tKey);
            }
            for(const auto &tab : tSec.tabs)
            {
                for(const auto &sKey : tab.sectionKeys)
                {
                    hiddenToParent[sKey] = &sections.at(tKey);
                }
            }
        }

        // Register one ImCmd command per visible parameter.
        // Name format: "? ParamName (opener->name)" — opener carries the part number
        // for tab-embedded sections.  A set prevents any remaining duplicates.
        std::set<std::string> seen;

        auto addParamCmds = [&](const SectionDef &sec, SectionDef *opener)
        {
            for(auto *param : sec.params)
            {
                if (!param)
                {
                    continue;
                }
                const std::string pname = param->name();
                if (pname == HIDDEN_PARAM_NAME)
                {
                    continue;
                }
                std::string cmdName = "? " + pname + " (" + opener->name + ")";
                if (!seen.insert(cmdName).second)
                {
                    continue;
                }
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

        for(auto &[key, section] : sections)
        {
            SectionDef *opener = &section;
            if (section.hideFromPalette)
            {
                auto it = hiddenToParent.find(key);
                if (it != hiddenToParent.end())
                {
                    opener = it->second;
                }
            }
            addParamCmds(section, opener);
            for(auto &sub : section.subSections)
            {
                addParamCmds(sub, opener);
            }
        }
    }

    // Scrollable canvas state
    ImVec2 scrollOfs  = {0.0f, 0.0f};  // canvas scroll (pixels)
    ImVec2 canvasMax  = {0.0f, 0.0f};  // running bounding box of open windows
    struct DragState { bool active = false; float mouseAnchor = 0, scrollAnchor = 0; };
    DragState vDrag, hDrag;
    constexpr float kSbW = 13.0f;       // scrollbar thickness


    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);

        // Mouse / scrollbar setup
        // These are computed from last-frame canvasMax (1-frame lag is fine).
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
        ImGui_ImplGlfw_NewFrame();   // updates display size, DPI scale, queues mouse events

        // Get raw mouse position directly from GLFW.
        // Do NOT read io.MousePos here: it is stale (set by the previous NewFrame) and
        // may be {-FLT_MAX,-FLT_MAX} from our own drag-blocking, causing the drag
        // computation to snap the view to position 0 whenever the mouse is stationary.
        ImVec2 rawMouse;
        {
            double mx, my;
            glfwGetCursorPos(window, &mx, &my);
            const ImGuiIO& io0 = ImGui::GetIO();
            rawMouse = { (float)mx * io0.DisplayFramebufferScale.x,
                         (float)my * io0.DisplayFramebufferScale.y };
        }
        // rawMouse is valid whenever the cursor is inside the framebuffer area.
        const bool rawValid = rawMouse.x >= 0.f && rawMouse.x < fw
                           && rawMouse.y >= 0.f && rawMouse.y < fh;

        const bool overVSb = needV && rawValid && rawMouse.x >= fw - kSbW;
        const bool overHSb = needH && rawValid && rawMouse.y >= fh - kSbW;
        {
            // Inject the adjusted mouse position as the LAST event in the queue so
            // that ImGui::NewFrame() uses it instead of the raw GLFW position.
            auto &io = ImGui::GetIO();
            if ((overVSb || overHSb || vDrag.active || hDrag.active) && rawValid)
            {
                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);  // hide from ImGui during scrollbar ops
            }
            else if (rawValid)
            {
                io.AddMousePosEvent(rawMouse.x + scrollOfs.x, rawMouse.y + scrollOfs.y);
            }
            else
            {
                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            }
        }

        ImGui::NewFrame();
        if (ed.highlightTimer > 0.f)
        {
            ed.highlightTimer -= ImGui::GetIO().DeltaTime;
        }

        // Sidebar -- fixed at screen position (0,0), independent of scroll
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(SidebarState::kWidth, (float)display_h), ImGuiCond_Always);
        if (ImGui::Begin("##Sidebar",
            nullptr,
            ImGuiWindowFlags_NoMove
            | ImGuiWindowFlags_NoResize
            | ImGuiWindowFlags_NoTitleBar
            | ImGuiWindowFlags_NoBringToFrontOnFocus
            | ImGuiWindowFlags_NoCollapse))
        {
            renderSidebar(ed, sections);
        }
        ImGui::End();

        // Shift the main viewport origin to match the current scroll offset.
        // This controls the clip rect applied to every Begin() call:
        //   clip = [Pos, Pos + Size] = [scrollOfs, scrollOfs + {fw, fh}]
        // so only content inside the visible scroll window is rendered.
        // ImGui::Render() reads viewport->Pos into DrawData->DisplayPos automatically,
        // so the OpenGL renderer maps that region correctly to screen coordinates.
        // WorkPos is set during NewFrame() and is NOT affected by our Pos change here.
        ImGui::GetMainViewport()->Pos = ImVec2(scrollOfs.x, scrollOfs.y);
        // Inflate work area so windows can auto-size beyond the visible screen region.
        ImGui::GetMainViewport()->WorkSize = {10000.f, 10000.f};
        canvasMax = {fw, fh};   // reset; grows during section rendering below

        // Apply MIDI receive results on the main thread (Lua calls safe here)
        {
            for (auto it = ed.pendingReceives.begin(); it != ed.pendingReceives.end();)
            {
                {
                    std::lock_guard<std::mutex> lock(ed.pendingMutex);
                    if (it->data.empty())
                    {
                        ++it;
                        continue;
                    }
                }
                auto msgs = it->handler(it->data);
                for (const auto& msg : msgs)
                {
                    if (!msg.id.empty())
                    {
                        valueChanged(ed, msg);
                    }
                }
                it = ed.pendingReceives.erase(it);
            }
            if (ed.pendingReceives.empty())
            {
                ed.isReceiving.store(false);
                if (ed.saveSysex.phase == I7Ed::SaveSysexState::Phase::ReadMsb)
                {
                    // MSB received -- determine tone type and receive tone sections
                    const std::string msbId = ed.saveSysex.partPrefix.empty()
                        ? ""
                        : [&]() -> std::string
                        {
                            // "Part 01 " -> partNr=1 -> "PRM-_PRF-_FP1-NEFP_PAT_BS_MSB"
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

        // Indeterminate sweep bar at the top while receiving
        if (ed.isReceiving.load())
        {
            const float elapsed = std::chrono::duration<float>(
                std::chrono::steady_clock::now() - ed.receiveStartTime).count();
            constexpr float kBarH   = 3.f;
            constexpr float kSegLen = 0.25f;   // segment width as fraction of bar
            constexpr float kPeriod = 1.5f;    // seconds per full sweep
            const float pos   = std::fmod(elapsed / kPeriod, 1.0f + kSegLen) - kSegLen;
            const float lFrac = std::clamp(pos,           0.0f, 1.0f);
            const float rFrac = std::clamp(pos + kSegLen, 0.0f, 1.0f);
            const float W     = static_cast<float>(display_w);
            auto* dl = ImGui::GetBackgroundDrawList();
            dl->AddRectFilled(
                {scrollOfs.x,               scrollOfs.y},
                {scrollOfs.x + W,           scrollOfs.y + kBarH},
                IM_COL32(80, 10, 10, 200));
            dl->AddRectFilled(
                {scrollOfs.x + lFrac * W,   scrollOfs.y},
                {scrollOfs.x + rFrac * W,   scrollOfs.y + kBarH},
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

        // File dialog for Save SysEx
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
                // Phase 1: receive only NEFP_PAT_BS_MSB to determine tone type.
                int n = std::stoi(ed.saveSysex.partPrefix.substr(5, 2));
                std::string msbId = "PRM-_PRF-_FP" + std::to_string(n) + "-NEFP_PAT_BS_MSB";
                SectionDef::FGetReceiveSysex msbGetter = [&ed, msbId]()
                    -> std::vector<RequestMessage>
                {
                    sol::function fn = ed.lua["CreateReceiveMessageForLeafId"];
                    sol::object obj = fn(msbId);
                    if (!obj.valid() || obj.get_type() == sol::type::nil)
                    {
                        return {};
                    }
                    return {obj.as<RequestMessage>()};
                };
                ed.saveSysex.phase = I7Ed::SaveSysexState::Phase::ReadMsb;
                triggerReceive(ed, {msbGetter});
            }
            ImGuiFileDialog::Instance()->Close();
        }

        for(auto &sectionPair : sections)
        {
            auto& section = sectionPair.second;
            if (!section.isOpen)
            {
                continue;
            }
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
                    renderSection(section, ed);
                    for(auto &subSection : section.subSections)
                    {
                        renderSection(subSection, ed);
                    }
                }
                ImGui::End();
            }
        }

        // Mouse-wheel scroll (only when not hovering any ImGui window)
        if (!ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
        {
            const auto &io2 = ImGui::GetIO();
            scrollOfs.x -= io2.MouseWheelH * 30.0f;
            scrollOfs.y -= io2.MouseWheel  * 30.0f;
        }

        const bool lmb = ImGui::GetIO().MouseDown[0];

        // Active drags end only when the mouse button is released, regardless of
        // whether needV/needH is currently true (canvasMax can oscillate by 1px).
        // New drags only start when needV/needH is confirmed.
        if (vDrag.active)
        {
            if (!lmb)
            {
                vDrag.active = false;
            }
            else
            {
                float scrollPerPx = maxScrollY / std::max(1.0f, vTrackLen - vThumbLen);
                scrollOfs.y = std::clamp(vDrag.scrollAnchor + (rawMouse.y - vDrag.mouseAnchor) * scrollPerPx,
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
            if (!lmb)
            {
                hDrag.active = false;
            }
            else
            {
                float scrollPerPx = maxScrollX / std::max(1.0f, hTrackLen - hThumbLen);
                scrollOfs.x = std::clamp(hDrag.scrollAnchor + (rawMouse.x - hDrag.mouseAnchor) * scrollPerPx,
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

        // Draw scrollbars (recompute thumb positions from final scrollOfs)
        {
            const float vtl2 = (needV && maxScrollY > 0.0f)
                                ? (vTrackLen - vThumbLen) * scrollOfs.y / maxScrollY : 0.0f;
            const float htl2 = (needH && maxScrollX > 0.0f)
                                ? (hTrackLen - hThumbLen) * scrollOfs.x / maxScrollX : 0.0f;
            constexpr ImU32 kTrack = IM_COL32( 30, 30, 30, 220);
            constexpr ImU32 kThumb = IM_COL32(120,120,120, 220);
            constexpr ImU32 kDrag  = IM_COL32(200,200,200, 255);
            auto *dl = ImGui::GetForegroundDrawList();
            if (needV)
            {
                dl->AddRectFilled({fw - kSbW + scrollOfs.x,        scrollOfs.y},
                                  {fw        + scrollOfs.x, fh - (needH ? kSbW : 0.f) + scrollOfs.y}, kTrack);
                dl->AddRectFilled({fw - kSbW + scrollOfs.x, vtl2        + scrollOfs.y},
                                  {fw        + scrollOfs.x, vtl2 + vThumbLen + scrollOfs.y}, vDrag.active ? kDrag : kThumb);
            }
            if (needH)
            {
                dl->AddRectFilled({       scrollOfs.x, fh - kSbW + scrollOfs.y},
                                  {fw - (needV ? kSbW : 0.f) + scrollOfs.x, fh + scrollOfs.y}, kTrack);
                dl->AddRectFilled({htl2        + scrollOfs.x, fh - kSbW + scrollOfs.y},
                                  {htl2 + hThumbLen + scrollOfs.x, fh   + scrollOfs.y}, hDrag.active ? kDrag : kThumb);
            }
            if (needV && needH)
            {
                dl->AddRectFilled({fw - kSbW + scrollOfs.x, fh - kSbW + scrollOfs.y},
                                  {fw        + scrollOfs.x, fh        + scrollOfs.y}, kTrack);
            }
        }

        renderMidiActivityLeds(display_w, display_h, ed.midiSendTimeNs, ed.midiRecvTimeNs,
                               {scrollOfs.x, scrollOfs.y});
        ImGui::Render();
        // DisplayPos is already set to scrollOfs by ImGui::Render() via viewport->Pos.
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);

        // Throttle when idle: avoids spinning while keeping the app
        // immediately responsive on any input event.
        {
            const bool imguiActive = ImGui::IsAnyItemActive() || ImGui::IsAnyItemHovered();
            const int64_t nowNs    = std::chrono::steady_clock::now().time_since_epoch().count();
            constexpr int64_t kLedFadeNs = 200'000'000LL; // slightly longer than kFade (0.15 s)
            const bool ledsActive  = (nowNs - ed.midiSendTimeNs.load(std::memory_order_relaxed)) < kLedFadeNs
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
