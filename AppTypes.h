#pragma once

#include <string>
#include <vector>
#include <list>
#include <deque>
#include <mutex>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <memory>
#include "ParameterDef.h"
#include "Midi.h"
#include "Com.h"
#include "imgui_notifications.h"
#include <sol/sol.hpp>

#define HIDDEN_PARAM_NAME "__HIDDEN__"

struct Args
{
    bool printHelp   = false;
    std::string mainLuaFilePath;
    std::string midiInName;
    std::string midiOutName;
    bool verbose = false;
};

struct PendingReceive
{
    RequestMessage::FOnMessageReceived handler;
    RequestMessage::FOnDone onDone;
    Bytes data;
    std::deque<Bytes> dataQueue;
    bool multiResponse = false;
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
    float width      = 250.0f;
    bool  isResizing = false;
    std::vector<std::string> inPortNames;
    std::vector<std::string> outPortNames;
    int selectedInPort  = -1;
    int selectedOutPort = -1;
    int deviceId = 16;
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
    int receiveTotalCount = 0;
    std::list<std::shared_ptr<PendingReceive>> pendingReceives;
    std::mutex pendingMutex;
    NotificationQueue notifications;
    std::atomic<int64_t> midiSendTimeNs{0};
    std::atomic<int64_t> midiRecvTimeNs{0};
    // param search highlight
    std::string highlightParamId;
    float       highlightTimer = 0.f;
    // navigate to tab/accordion on param search
    std::string navigateOpenerName;
    std::string navigateTabLabel;
    std::string navigateAccordionLabel;
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
        std::string partPrefix;
        std::vector<std::string> tonePrefixes;
    } saveSysex;
    SidebarState sidebar;
    ImTextureID knobTexture       = 0;
    int         knobAtlasFrames  = 128;
    int         knobAtlasCols    = 16;
    ImTextureID sliderHandleTex  = 0;
    ImTextureID drawbarTexBk     = 0;
    ImTextureID drawbarTexWt     = 0;
    ImTextureID drawbarTexBr     = 0;
};
