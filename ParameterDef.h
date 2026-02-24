#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>
#include <functional>

#define PARAM_TYPE_RANGE     "range"
#define PARAM_TYPE_SELECTION "select"
#define PARAM_TYPE_TOGGLE    "toggle"
#define PARAM_TYPE_ENVELOPE  "envelope"
#define PARAM_TYPE_STEP_LFO  "steplfo"

typedef std::vector<unsigned char> Bytes;

struct ParameterDef
{
    typedef std::list<ParameterDef*> Params;
    typedef std::function<int(float)> FToI7Value;
    typedef std::function<int(float)> FToGuiValue;
    typedef std::function<float()> FFloatGetter;
    typedef std::function<std::string()> FStringGetter;
    typedef std::function<std::vector<unsigned char>(float)> FSetValue;
    typedef std::map<int, std::string> SelectionOptions;
    std::string id;
    FStringGetter name;
    std::string format = "%.0f";
    std::string type;
    FToI7Value toI7Value;
    FToGuiValue toGuiValue;
    SelectionOptions options;
    std::string stringValue;
    float value = 0.0f;
    FFloatGetter min = [](){ return 0; };
    FFloatGetter max = [](){ return 127; };
    FSetValue setValue;
    // PARAM_TYPE_ENVELOPE only
    std::vector<std::string> levelIds;
    std::vector<std::string> timeIds;
    bool sustainSegment = false;
    // PARAM_TYPE_STEP_LFO only
    std::string stepTypeId;
    std::vector<std::string> stepIds;
};

struct ValueChangedMessage
{
    std::string id;
    float i7Value = 0;
};

struct RequestMessage 
{
    typedef std::function<std::vector<ValueChangedMessage>(std::vector<unsigned char>)> FOnMessageReceived;
    Bytes sysex;
    FOnMessageReceived onMessageReceived;
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
    bool isOpen = false;
    // Tab-group section (tabs over per-partial sections)
    struct TabEntry
    {
        std::string              label;
        std::vector<std::string> sectionKeys;
    };
    std::string            tabCommonKey;    // optional section rendered above the tab bar
    std::vector<TabEntry>  tabs;            // non-empty → render as tab bar
    bool                   hideFromPalette = false; // true for sections embedded in a tab view
};
