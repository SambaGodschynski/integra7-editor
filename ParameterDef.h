#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>
#include <functional>
#include "Com.h"

#define PARAM_TYPE_RANGE     "range"
#define PARAM_TYPE_SELECTION "select"
#define PARAM_TYPE_TOGGLE    "toggle"
#define PARAM_TYPE_ENVELOPE  "envelope"
#define PARAM_TYPE_STEP_LFO  "steplfo"
#define PARAM_TYPE_ACTION    "action"
#define PARAM_TYPE_SAVE_SYSEX "savesysex"
#define PARAM_TYPE_LOAD_SYSEX "loadsysex"
#define PARAM_TYPE_SOLO_TOGGLE "solotoggle"
#define PARAM_TYPE_VSLIDER     "vslider"
#define PARAM_TYPE_NEWLINE     "newline"
#define PARAM_TYPE_SEPARATOR   "separator"
#define PARAM_TYPE_INPUTTEXT   "inputtext"

struct RequestMessage; // forward declaration for FGetAction

struct ParameterDef
{
    typedef std::list<ParameterDef*> Params;
    typedef std::function<int(float)> FToI7Value;
    typedef std::function<float(float)> FToGuiValue;
    typedef std::function<float()> FFloatGetter;
    typedef std::function<std::string()> FStringGetter;
    typedef std::function<std::vector<unsigned char>(float)> FSetValue;
    typedef std::map<int, std::string> SelectionOptions;
    typedef std::function<std::vector<RequestMessage>()> FGetAction;
    typedef std::function<std::vector<unsigned char>(std::string)> FSetStringValue;
    typedef std::function<SelectionOptions()> FGetOptions;
    std::string id;
    FStringGetter name;
    std::string format = "%.0f";
    std::string type;
    FToI7Value toI7Value;
    FToGuiValue toGuiValue;
    SelectionOptions options;
    FGetOptions optionsFn;
    std::string stringValue;
    float value = 0.0f;
    FFloatGetter min = []()
    {
        return 0;
    };
    FFloatGetter max = []()
    {
        return 127;
    };
    FSetValue setValue;
    // Optional: if set, called every frame to override param->value (no SysEx triggered)
    FFloatGetter valueOverride;
    // PARAM_TYPE_ACTION only
    FGetAction getAction;
    // PARAM_TYPE_INPUTTEXT only
    FSetStringValue setStringValue;
    FStringGetter stringValueGetter;
    // PARAM_TYPE_SAVE_SYSEX only
    std::string partPrefix;
    // PARAM_TYPE_SOLO_TOGGLE only
    std::string linkedParamId;
    float linkedValue = 0.0f;
    // PARAM_TYPE_RANGE / PARAM_TYPE_VSLIDER optional size override
    float size = 0.0f;
    // PARAM_TYPE_RANGE knob display options
    bool noTitle = false;
    bool noInput = false;
    // PARAM_TYPE_ENVELOPE only
    std::vector<std::string> levelIds;
    std::vector<std::string> timeIds;
    bool sustainSegment = false;
    // PARAM_TYPE_STEP_LFO only
    std::string stepTypeId;
    std::vector<std::string> stepIds;
    // PARAM_TYPE_RANGE drawbar display: returns "bk", "wt", or "br"; null = normal knob
    FStringGetter drawbarColor;
};

struct ValueChangedMessage
{
    std::string id;
    float i7Value = 0;
};

struct RequestMessage
{
    typedef std::function<std::vector<ValueChangedMessage>(std::vector<unsigned char>)> FOnMessageReceived;
    typedef std::function<std::vector<RequestMessage>()> FOnDone;
    Bytes sysex;
    FOnMessageReceived onMessageReceived;
    FOnDone onDone;  // called after response is processed; returned requests are queued next
    bool     multiResponse = false;
    int      receiveGapMs = 0;      // 0 = use default (300 ms)
    uint32_t stopOnAddr   = 0;      // exit multiResponse loop immediately when this SysEx addr arrives
};

struct SectionDef
{
    typedef std::map<std::string, SectionDef> NamedSections;
    typedef std::list<SectionDef> Sections;
    typedef std::function<std::vector<RequestMessage>()> FGetReceiveSysex;
    Sections subSections;
    std::string name;
    ParameterDef::Params params;
    FGetReceiveSysex getReceiveSysex;
    FGetReceiveSysex onOpen;  // called once when accordion header is clicked open
    bool isOpen = false;
    // Tab-group section (tabs over per-partial sections)
    struct SectionRef
    {
        std::string key;
        std::string accordionLabel; // non-empty: wrap in CollapsingHeader with this label
    };
    struct TabEntry
    {
        std::string              label;
        std::vector<SectionRef>  sectionKeys;
    };
    std::string            tabCommonKey;    // optional section rendered above the tab bar
    std::vector<TabEntry>  tabs;            // non-empty → render as tab bar
    bool                   hideFromPalette = false; // true for sections embedded in a tab view
    bool                   accordion = false;       // true → subSections render as collapsing headers
    std::string            layout;                 // "" = generic, "eq3band" = 3-band EQ columns
};
