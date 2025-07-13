#pragma once

#include <string>
#include <list>
#include <map>
#include <list>
#include <functional>

#define PARAM_TYPE_RANGE "range"
#define PARAM_TYPE_SELECTION "select"

struct ParameterDef
{
    typedef std::list<ParameterDef> Params;
    typedef std::function<int(float)> FToI7Value;
    typedef std::map<int, std::string> SelectionOptions;
    std::string id;
    std::string name;
    std::string format = "%.0f";
    std::string type;
    FToI7Value toI7Value;
    SelectionOptions options;
    std::string stringValue;
    float value = 0.0f;
    float min = 0.0f;
    float max = 127.0f;
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
