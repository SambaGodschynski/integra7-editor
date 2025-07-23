#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>
#include <functional>

#define PARAM_TYPE_RANGE "range"
#define PARAM_TYPE_SELECTION "select"
#define PARAM_TYPE_TOGGLE "toggle"

struct ParameterDef
{
    typedef std::list<ParameterDef> Params;
    typedef std::function<int(float)> FToI7Value;
    typedef std::function<float()> FFloatGetter;
    typedef std::function<std::string()> FStringGetter;
    typedef std::function<std::vector<unsigned char>(float)> FSetValue;
    typedef std::map<int, std::string> SelectionOptions;
    std::string id;
    FStringGetter name;
    std::string format = "%.0f";
    std::string type;
    FToI7Value toI7Value;
    SelectionOptions options;
    std::string stringValue;
    float value = 0.0f;
    FFloatGetter min = [](){ return 0; };
    FFloatGetter max = [](){ return 127; };
    FSetValue setValue;
};


struct SectionDef
{
    typedef std::map<std::string, SectionDef> NamedSections;
    typedef std::list<SectionDef> Sections;
    Sections subSections;
    std::string name;
    ParameterDef::Params params;
    bool isOpen = false;
};
