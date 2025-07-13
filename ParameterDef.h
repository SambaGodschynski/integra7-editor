#pragma once

#include <string>
#include <list>
#include <map>
#include <list>
#include <functional>
struct ParameterDef
{
    typedef std::list<ParameterDef> Params;
    typedef std::function<int(float)> FToI7Value;
    std::string id;
    std::string name;
    std::string format = "%.0f";
    FToI7Value toI7Value;
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
