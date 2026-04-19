#include "LuaBridge.h"

std::string getDefaultValue(const ParameterDef::SelectionOptions& options, int defaultValue)
{
    const auto& it = options.find(defaultValue);
    if (it == options.end())
    {
        return std::string();
    }
    return it->second;
}

void getSection(I7Ed& ed, sol::table& lua_table, SectionDef& outSectionDef)
{
    outSectionDef.name = require_key<std::string>(lua_table, "name");
    outSectionDef.getReceiveSysex = optional_key<SectionDef::FGetReceiveSysex>(
        lua_table, "getReceiveValueSysex", nullptr);

    if (lua_table["params"] != sol::nil)
    {
        sol::table params = lua_table["params"];
        for (auto& luaParamPair : params)
        {
            auto param = std::make_shared<ParameterDef>();
            auto luaParam = luaParamPair.second.as<sol::table>();
            sol::object paramName = luaParam["name"];
            sol::object paramId   = luaParam["id"];
            // If name is a plain string, capture it by value.
            // If name is a Lua function (dynamic, e.g. __HIDDEN__ logic), keep it
            // callable so the render loop can query it every frame.
            // sol::state lives in I7Ed for the entire app lifetime, so holding a
            // sol::function here is safe -- the lua_State* never becomes invalid.
            {
                if (paramName.get_type() == sol::type::function)
                {
                    sol::function fn = paramName.as<sol::function>();
                    param->name = [fn]() mutable { return fn.call<std::string>(); };
                }
                else
                {
                    std::string resolvedName = paramName.as<std::string>();
                    param->name = [resolvedName]() { return resolvedName; };
                }
            }
            param->id   = paramId.as<std::string>();
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
            else if (param->type == PARAM_TYPE_SOLO_TOGGLE)
            {
                param->linkedParamId = require_key<std::string>(luaParam, "linkedParamId");
                param->linkedValue   = optional_key<float>(luaParam, "linkedValue", 0.0f);
            }
            else if (param->type == PARAM_TYPE_NEWLINE)
            {
                // no fields needed
            }
            else if (param->type == PARAM_TYPE_INPUTTEXT)
            {
                param->setStringValue = require_key<ParameterDef::FSetStringValue>(luaParam, "setStringValue");
                param->stringValueGetter = optional_key<ParameterDef::FStringGetter>(luaParam, "stringValueGetter", nullptr);
                param->stringValue = optional_key<std::string>(luaParam, "stringValue", "");
            }
            else
            {
                param->setValue = require_key<ParameterDef::FSetValue>(luaParam, "setValue");
            }

            param->drawbarColor = optional_key<ParameterDef::FStringGetter>(luaParam, "drawbarColor", nullptr);
            param->size      = optional_key<float>(luaParam, "size", 0.0f);
            param->noTitle   = optional_key<bool>(luaParam, "noTitle", false);
            param->noInput   = optional_key<bool>(luaParam, "noInput", false);
            param->min       = optional_key(luaParam, "min",       param->min);
            param->max       = optional_key(luaParam, "max",       param->max);
            param->format    = optional_key(luaParam, "format",    param->format);
            param->value     = optional_key(luaParam, "default",   param->min ? param->min() : 0);
            param->toI7Value     = optional_key<ParameterDef::FToI7Value>    (luaParam, "toI7Value",     nullptr);
            param->toGuiValue    = optional_key<ParameterDef::FToGuiValue>   (luaParam, "toGuiValue",    nullptr);
            param->valueOverride = optional_key<ParameterDef::FFloatGetter>  (luaParam, "valueOverride", nullptr);
            param->options   = optional_key<ParameterDef::SelectionOptions>(luaParam, "options",
                                    ParameterDef::SelectionOptions());

            if (!param->options.empty())
            {
                param->stringValue = getDefaultValue(param->options, (int)param->value);
            }
            else
            {
                sol::optional<sol::function> optFnLua = luaParam["options"];
                if (optFnLua && optFnLua->valid())
                {
                    sol::function fn = *optFnLua;
                    param->optionsFn = [fn]() -> ParameterDef::SelectionOptions
                    {
                        try
                        {
                            sol::table luaTable = fn();
                            ParameterDef::SelectionOptions opts;
                            for (const auto& kv : luaTable)
                            {
                                if (kv.first.get_type()  == sol::type::number
                                 && kv.second.get_type() == sol::type::string)
                                {
                                    opts[kv.first.as<int>()] = kv.second.as<std::string>();
                                }
                            }
                            return opts;
                        }
                        catch (...)
                        {
                            return {};
                        }
                    };
                }
            }

            auto [it, inserted] = ed.parameterDefs.emplace(param->id, param);
            if (!inserted)
            {
                // ID already registered: reuse the existing ParameterDef so
                // both sections share the same instance (no dangling pointer).
                outSectionDef.params.push_back(it->second.get());
            }
            else
            {
                outSectionDef.params.push_back(param.get());
            }
        }
    }

    if (lua_table["grp"] != sol::nil)
    {
        sol::table luaSubSections = lua_table["grp"];
        for (const auto& luaSubSectionObj : luaSubSections)
        {
            sol::table luaSubSection = luaSubSectionObj.second;
            SectionDef subSection;
            getSection(ed, luaSubSection, subSection);
            outSectionDef.subSections.push_back(subSection);
        }
    }

    if (lua_table["isOpen"] != sol::nil)
    {
        outSectionDef.isOpen = lua_table["isOpen"];
    }

    if (lua_table["hideFromPalette"] != sol::nil)
    {
        outSectionDef.hideFromPalette = lua_table["hideFromPalette"];
    }

    if (lua_table["accordion"] != sol::nil)
    {
        outSectionDef.accordion = lua_table["accordion"];
    }

    if (lua_table["layout"] != sol::nil)
    {
        outSectionDef.layout = lua_table["layout"].get<std::string>();
    }

    if (lua_table["tabs"] != sol::nil)
    {
        if (lua_table["tabCommonKey"] != sol::nil)
        {
            outSectionDef.tabCommonKey = lua_table["tabCommonKey"].get<std::string>();
        }
        sol::table luaTabs = lua_table["tabs"];
        for (const auto& tabPair : luaTabs)
        {
            sol::table luaTab = tabPair.second.as<sol::table>();
            SectionDef::TabEntry entry;
            entry.label = luaTab["label"].get<std::string>();
            sol::table keys = luaTab["keys"];
            for (const auto& kp : keys)
            {
                SectionDef::SectionRef ref;
                if (kp.second.get_type() == sol::type::string)
                {
                    ref.key = kp.second.as<std::string>();
                }
                else if (kp.second.get_type() == sol::type::table)
                {
                    sol::table kt = kp.second.as<sol::table>();
                    ref.key = kt["key"].get<std::string>();
                    if (kt["accordion"] != sol::nil)
                    {
                        ref.accordionLabel = kt["accordion"].get<std::string>();
                    }
                }
                entry.sectionKeys.push_back(ref);
            }
            outSectionDef.tabs.push_back(entry);
        }
    }
}

void getDefs(I7Ed& ed, SectionDef::NamedSections& outNamedSections)
{
    sol::table mainSections = ed.lua["Main"];
    for (auto& luaSectionPair : mainSections)
    {
        sol::table luaSection = luaSectionPair.second.as<sol::table>();
        SectionDef sectionDef;
        getSection(ed, luaSection, sectionDef);
        outNamedSections.insert({luaSectionPair.first.as<std::string>(), sectionDef});
    }
}
