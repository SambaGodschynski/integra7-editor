#pragma once

#include "AppTypes.h"
#include <sol/sol.hpp>
#include <stdexcept>

// ── Template helpers (must live in header) ───────────────────────────────────

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

// ── Non-template declarations ────────────────────────────────────────────────

std::string getDefaultValue(const ParameterDef::SelectionOptions& options, int defaultValue);
void getSection(I7Ed& ed, sol::table& lua_table, SectionDef& outSectionDef);
void getDefs(I7Ed& ed, SectionDef::NamedSections& outNamedSections);
