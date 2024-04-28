#pragma once
// js\model\model_id.js

#include <unordered_map>
#include "Integra7Defs.h"
#include <string>

namespace i7
{
    typedef std::unordered_map<std::string, UInt> ModelIdMap;
    extern const ModelIdMap modelIdMap;
    extern UInt getModelIdAddress(const std::string &modelId);
}