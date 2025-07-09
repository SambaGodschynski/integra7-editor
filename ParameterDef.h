#pragma once

#include <string>
#include <list>
#include <map>

struct ParameterDef
{
    typedef std::list<ParameterDef> Params;
    std::string id;
    std::string name;
    float value = 0; // TODO: find best type, maybe union
};


struct SectionDef
{
    typedef std::map<std::string, SectionDef> Sections;
    std::string name;
    ParameterDef::Params params;
    bool isOpen = true;
};
