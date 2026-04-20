#include "SysexIO.h"
#include "imgui.h"          // ImVec4
#include <sol/sol.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <unordered_map>

void sendMessage(I7Ed& ed, const Bytes& message)
{
    if (message.empty())
    {
        return;
    }
    ed.midi.sendMessage(message);
}

void valueChanged(I7Ed& ed, const ParameterDef& paramDef)
{
    try
    {
        if (ed.args.verbose)
        {
            std::cout << "[changed] " << paramDef.id << std::endl;
        }
        int i7Value = (int)paramDef.value;
        if (paramDef.toI7Value)
        {
            i7Value = paramDef.toI7Value(paramDef.value);
        }
        auto sysex = paramDef.setValue(i7Value);
        sendMessage(ed, sysex);
    }
    catch (const sol::error& error)
    {
        std::cerr << error.what() << std::endl;
    }
}

void valueChanged(I7Ed& ed, const ValueChangedMessage& vcMessage)
{
    auto paramDef = getParameterDef(ed, vcMessage.id);
    if (paramDef == nullptr)
    {
        std::cerr << "unknown parameter change received: " << vcMessage.id << std::endl;
        return;
    }
    float guiValue = vcMessage.i7Value;
    if (paramDef->toGuiValue)
    {
        guiValue = paramDef->toGuiValue(vcMessage.i7Value);
    }
    paramDef->value = guiValue;
    {
        const ParameterDef::SelectionOptions resolvedOpts =
            paramDef->optionsFn ? paramDef->optionsFn() : paramDef->options;
        auto it = resolvedOpts.find((int)paramDef->value);
        if (it != resolvedOpts.end())
        {
            paramDef->stringValue = it->second;
        }
    }
}

ParameterDef* getParameterDef(I7Ed& ed, const std::string& id)
{
    auto mapIt = ed.parameterDefs.find(id);
    if (mapIt == ed.parameterDefs.end())
    {
        return nullptr;
    }
    return mapIt->second.get();
}

std::vector<RequestMessage> buildParamRequests(
    I7Ed& ed, const std::vector<ParameterDef*>& params)
{
    sol::function fn = ed.lua["CreateReceiveMessageForLeafId"];
    std::vector<RequestMessage> result;

    auto tryAdd = [&](const std::string& id)
    {
        if (!ed.parameterDefs.count(id))
        {
            return;
        }
        sol::object obj = fn(id);
        if (!obj.valid() || obj.get_type() == sol::type::nil)
        {
            return;
        }
        result.push_back(obj.as<RequestMessage>());
    };

    for (const auto* param : params)
    {
        if (!param)
        {
            continue;
        }
        if (param->type == PARAM_TYPE_RANGE
         || param->type == PARAM_TYPE_SELECTION
         || param->type == PARAM_TYPE_TOGGLE
         || param->type == PARAM_TYPE_VSLIDER)
        {
            tryAdd(param->id);
        }
        else if (param->type == PARAM_TYPE_ENVELOPE)
        {
            for (const auto& id : param->levelIds) { tryAdd(id); }
            for (const auto& id : param->timeIds)  { tryAdd(id); }
        }
        else if (param->type == PARAM_TYPE_STEP_LFO)
        {
            if (!param->stepTypeId.empty()) { tryAdd(param->stepTypeId); }
            for (const auto& id : param->stepIds) { tryAdd(id); }
        }
    }
    return result;
}

SectionDef::FGetReceiveSysex makeParamOnlyGetter(I7Ed& ed, const SectionDef& sec)
{
    std::vector<ParameterDef*> params(sec.params.begin(), sec.params.end());
    for (const auto& sub : sec.subSections)
    {
        for (auto* p : sub.params) { params.push_back(p); }
    }
    return [&ed, params]() -> std::vector<RequestMessage>
    {
        return buildParamRequests(ed, params);
    };
}

void enqueueRequest(I7Ed& ed, const RequestMessage& req)
{
    if (req.multiResponse)
    {
        ed.pendingReceives.push_back({
            .handler       = req.onMessageReceived,
            .onDone        = req.onDone,
            .multiResponse = true
        });
        ed.midi.sendAndReceive(req.sysex, &ed.pendingReceives.back(),
            [&ed](Bytes received, void* userData)
            {
                std::lock_guard<std::mutex> lock(ed.pendingMutex);
                if (userData == nullptr) return;
                PendingReceive* pData = (PendingReceive*)userData;
                pData->dataQueue.push_back(std::move(received));
            }, true, req.receiveGapMs > 0 ? req.receiveGapMs : 300, req.stopOnAddr);
    }
    else
    {
        ed.pendingReceives.push_back({
            .handler = req.onMessageReceived,
            .onDone  = req.onDone
        });
        ed.midi.sendAndReceive(req.sysex, &ed.pendingReceives.back(),
            [&ed](Bytes received, void* userData)
            {
                if (received.empty()) return;
                std::lock_guard<std::mutex> lock(ed.pendingMutex);
                if (userData == nullptr) return;
                PendingReceive* pData = (PendingReceive*)userData;
                pData->data.swap(received);
            });
    }
}

void triggerReceive(I7Ed& ed, const std::vector<SectionDef::FGetReceiveSysex>& getters)
{
    if (ed.isReceiving.exchange(true))
    {
        return;
    }
    ed.receiveStartTime = std::chrono::steady_clock::now();

    for (const auto& getter : getters)
    {
        if (!getter) continue;
        for (const RequestMessage& req : getter())
        {
            enqueueRequest(ed, req);
        }
    }
    ed.receiveTotalCount = (int)ed.pendingReceives.size();
}

std::vector<std::string> getTonePrefixes(const std::string& partPrefix, int msb)
{
    switch (msb)
    {
        case 89: return {partPrefix + "SNA", partPrefix + "MFX"};
        case 95: return {partPrefix + "SN-S "};
        case 88: return {partPrefix + "SN-D "};
        case 87: return {partPrefix + "PCM-S "};
        case 86: return {partPrefix + "PCM-D "};
        default: return {};
    }
}

void saveSysexToFile(I7Ed& ed)
{
    if (!ed.pSections || ed.saveSysex.filepath.empty())
    {
        return;
    }
    std::ofstream file(ed.saveSysex.filepath, std::ios::binary);
    if (!file)
    {
        ed.notifications.push("Error: could not open file: " + ed.saveSysex.filepath,
                              ImVec4(1.f, 0.2f, 0.2f, 1.f));
        return;
    }
    int count = 0;

    auto writeParam = [&](const std::string& paramId)
    {
        auto it = ed.parameterDefs.find(paramId);
        if (it == ed.parameterDefs.end()) { return; }
        auto* p = it->second.get();
        if (!p->setValue) { return; }
        try
        {
            int i7v = p->toI7Value ? p->toI7Value(p->value) : (int)p->value;
            Bytes sysex = p->setValue(i7v);
            if (!sysex.empty() && sysex[0] == 0xF0)
            {
                file.write(reinterpret_cast<const char*>(sysex.data()),
                           static_cast<std::streamsize>(sysex.size()));
                ++count;
            }
        }
        catch (const sol::error& e)
        {
            std::cerr << "saveSysex: " << paramId << ": " << e.what() << std::endl;
        }
    };

    auto writeSection = [&](const SectionDef& sec)
    {
        for (const auto* param : sec.params)
        {
            if (!param) { continue; }
            if (param->type == PARAM_TYPE_RANGE
             || param->type == PARAM_TYPE_SELECTION
             || param->type == PARAM_TYPE_TOGGLE)
            {
                writeParam(param->id);
            }
            else if (param->type == PARAM_TYPE_ENVELOPE)
            {
                for (const auto& id : param->levelIds) { writeParam(id); }
                for (const auto& id : param->timeIds)  { writeParam(id); }
            }
            else if (param->type == PARAM_TYPE_STEP_LFO)
            {
                if (!param->stepTypeId.empty()) { writeParam(param->stepTypeId); }
                for (const auto& id : param->stepIds) { writeParam(id); }
            }
        }
    };

    auto keyMatches = [&](const std::string& key) -> bool
    {
        for (const auto& pfx : ed.saveSysex.tonePrefixes)
        {
            if (key.size() >= pfx.size() && key.substr(0, pfx.size()) == pfx)
            {
                return true;
            }
        }
        return false;
    };

    for (auto& [key, sec] : *ed.pSections)
    {
        if (!keyMatches(key)) { continue; }
        writeSection(sec);
        for (const auto& sub : sec.subSections)
        {
            writeSection(sub);
        }
    }
    file.close();
    std::string msg = "Saved " + std::to_string(count) + " SysEx to " + ed.saveSysex.filepath;
    ed.notifications.push(msg, ImVec4(0.2f, 1.f, 0.4f, 1.f), 5.f, 3.5f);
}

void loadSysexFromFile(I7Ed& ed)
{
    if (ed.loadSysex.filepath.empty())
    {
        return;
    }
    std::vector<ParameterDef*> allParams;
    allParams.reserve(ed.parameterDefs.size());
    for (auto& [id, param] : ed.parameterDefs)
    {
        allParams.push_back(param.get());
    }
    auto allRequests = buildParamRequests(ed, allParams);

    std::unordered_map<uint32_t, RequestMessage::FOnMessageReceived> addrMap;
    addrMap.reserve(allRequests.size());
    for (const auto& req : allRequests)
    {
        if (req.sysex.size() >= 11)
        {
            uint32_t addr = ((uint32_t)req.sysex[7] << 24)
                          | ((uint32_t)req.sysex[8] << 16)
                          | ((uint32_t)req.sysex[9]  << 8)
                          |  (uint32_t)req.sysex[10];
            addrMap[addr] = req.onMessageReceived;
        }
    }

    std::ifstream file(ed.loadSysex.filepath, std::ios::binary);
    if (!file)
    {
        ed.notifications.push("Error: could not open file: " + ed.loadSysex.filepath,
                              ImVec4(1.f, 0.2f, 0.2f, 1.f));
        return;
    }
    Bytes fileBytes((std::istreambuf_iterator<char>(file)), {});
    file.close();

    int count = 0;
    for (size_t i = 0; i < fileBytes.size(); )
    {
        if (fileBytes[i] != 0xF0) { ++i; continue; }
        auto endIt = std::find(fileBytes.begin() + (ptrdiff_t)i, fileBytes.end(),
                               (unsigned char)0xF7);
        if (endIt == fileBytes.end()) { break; }
        ++endIt;
        Bytes msg(fileBytes.begin() + (ptrdiff_t)i, endIt);

        if (msg.size() >= 12 && msg[6] == 0x12)
        {
            uint32_t addr = ((uint32_t)msg[7] << 24)
                          | ((uint32_t)msg[8] << 16)
                          | ((uint32_t)msg[9]  << 8)
                          |  (uint32_t)msg[10];
            auto it = addrMap.find(addr);
            if (it != addrMap.end())
            {
                auto vcMsgs = it->second(msg);
                for (const auto& vcm : vcMsgs)
                {
                    if (!vcm.id.empty())
                    {
                        valueChanged(ed, vcm);
                        auto* p = getParameterDef(ed, vcm.id);
                        if (p && p->setValue)
                        {
                            valueChanged(ed, *p);
                        }
                        ++count;
                    }
                }
            }
        }
        i = (size_t)(endIt - fileBytes.begin());
    }
    std::string msg = "Loaded " + std::to_string(count) + " params from " + ed.loadSysex.filepath;
    ed.notifications.push(msg, ImVec4(0.2f, 1.f, 0.4f, 1.f), 5.f, 3.5f);
}
