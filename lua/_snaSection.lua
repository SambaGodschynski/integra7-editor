require "math"
require "_snaData"
require "_sysex"
require "_com"
require "_model"

-- Drawbar colors for Hammond organ (MOD_PRM1-9 = 9 drawbars)
local kDrawbarColors = { "br", "br", "wt", "wt", "bk", "wt", "bk", "bk", "wt" }

local idInstNr = "PRM-_FPARTxxx_InstNr"
local instrumentNumberPartMap = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
local get = GetWrapper

local function i7offsetValue(offset)
    return function (guiValue)
        return math.tointeger(offset + guiValue)
    end
end

local function guiOffsetValue(offset)
    return function (i7Value)
        return math.tointeger(i7Value - offset)
    end
end

local function idTmpl(snaId)
    return "PRM-_FPARTxxx-_SNTONE-_SNTC-SNTC_" .. snaId
end

local function getInstrumentData(part)
    local idx = instrumentNumberPartMap[part]
    return SnaInstPresetData[idx]
end

local function modName(part, modNr)
    local instrumentData = getInstrumentData(part)
    if instrumentData == nil then
        return HideParam
    end
    if instrumentData.modData == MOD_TW or instrumentData.modData == MOD_SNAP then
        if instrumentData.modData[modNr] == nil then
            return HideParam
        end
        return instrumentData.modData[modNr].desc
    end
    local modKey = "mod"..tostring(modNr)
    if instrumentData[modKey] == nil then
        return HideParam
    end
    return instrumentData[modKey]
end

local function modMin(part, modNr)
    local instrumentData = getInstrumentData(part)
    if instrumentData == nil then
        return 0
    end
    local mod = instrumentData.modData[modNr]
    if mod == nil then
        return 0
    end
    if mod.id == ModVariationId and instrumentData.vari ~= nil then
        return instrumentData.vari.min
    end
    return mod.min
end

local function modMax(part, modNr)
    local instrumentData = getInstrumentData(part)
    if instrumentData == nil then
        return 127
    end
    local mod = instrumentData.modData[modNr]
    if mod == nil then
        return 127
    end
    if mod.id == ModVariationId and instrumentData.vari ~= nil then
        return instrumentData.vari.max
    end
    return mod.max
end

local function instrumentChange(part, index)
    instrumentNumberPartMap[part] = index
    local instrumentData = getInstrumentData(part)
    local lsb = instrumentData.lsb
    local pc = instrumentData.pc
    local lsbId = "PRM-_FPART" .. tostring(part) .. "-_SNTONE-_SNTC-SNTC_INST_BS_LSB"
    local lsbMessage = CreateSysexMessage(lsbId, lsb)
    local pcId = "PRM-_FPART" .. tostring(part) .. "-_SNTONE-_SNTC-SNTC_INST_BS_PC"
    local pcMessage = CreateSysexMessage(pcId, pc)
    local msg = Concat(lsbMessage, pcMessage)
    return msg
end

local snaTemplate = {
    name = "SN-A",
    getReceiveValueSysex = nil,
    grp =
    {
        {
            name = "Common",
            params =
            {
              {type="select", id=idTmpl("CATE"), name=get("Tone Category"), default=0, options = ToneCategories},
              {type="range", id=idTmpl("PHRASE_OCT"), name=get("Phrase Oct Shift"), min=get(-3), max=get(3), default=0, format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
              {type="range", id=idTmpl("TONE_LEVEL"), name=get("Tone Level"), min=get(0), max=get(127), default=127, format="%.0f"},
              {type="select", id=idTmpl("MONO_POLY"), name=get("Mono/Poly"), default=1, options = MonoPoly},
              {type="range", id=idTmpl("OCTAVE"), name=get("Oct Shift"), min=get(-3), max=get(3), default=0, format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
              {type="range", id=idTmpl("PORT_TIME"), name=get("Portamento Time Offset"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
              {type="range", id=idTmpl("VIB_RATE"), name=get("Vibrato Rate"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
              {type="range", id=idTmpl("VIB_DEPTH"), name=get("Vibrato Depth"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
              {type="range", id=idTmpl("VIB_DELAY"), name=get("Vibrato Delay"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
            }
        },
        {
            name = "Instrument",
            layout = "drawbars",
            params =
            {
              {type="select", id=idInstNr, name=get("Inst. Number"), default=1, options = SnaInstPresets},
              {type="range", id=idTmpl("MOD_PRM1")},
              {type="range", id=idTmpl("MOD_PRM2")},
              {type="range", id=idTmpl("MOD_PRM3")},
              {type="range", id=idTmpl("MOD_PRM4")},
              {type="range", id=idTmpl("MOD_PRM5")},
              {type="range", id=idTmpl("MOD_PRM6")},
              {type="range", id=idTmpl("MOD_PRM7")},
              {type="range", id=idTmpl("MOD_PRM8")},
              {type="range", id=idTmpl("MOD_PRM9")},
              {type="range", id=idTmpl("MOD_PRM10")},
              {type="range", id=idTmpl("MOD_PRM11")},
              {type="range", id=idTmpl("MOD_PRM12")},
              {type="range", id=idTmpl("MOD_PRM13")},
              {type="range", id=idTmpl("MOD_PRM14")},
              {type="range", id=idTmpl("MOD_PRM15")},
              {type="range", id=idTmpl("MOD_PRM16")},
              {type="range", id=idTmpl("MOD_PRM17")},
              {type="range", id=idTmpl("MOD_PRM18")},
              {type="range", id=idTmpl("MOD_PRM19")},
              {type="range", id=idTmpl("MOD_PRM20")},
              {type="range", id=idTmpl("MOD_PRM21")},
              {type="range", id=idTmpl("MOD_PRM22")},
            }
        }
    }
}

local function createInstrumentChangeMessage(partNr, instLsb, instPc)
    local msg = ValueChangedMessage.new()
    msg.id = CreateId(idInstNr, partNr)
    for index, preset in ipairs(SnaInstPresetData) do
        if preset.lsb == instLsb and preset.pc == instPc then
            instrumentNumberPartMap[partNr] = index
            msg.i7Value = index
            return msg
        end
    end
    return EmptyValueChangedMessage
end

function CreateSnaSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " SNA"
        local name = k
        local instLsb = nil
        local instPc = nil
        local snaData = DeepCopy(snaTemplate);
        local function handleInstrumentChange(instLsb, instPc)
            if instLsb==nil or instPc==nil then
                return EmptyValueChangedMessage
            end
            local l = instLsb
            local p = instPc
            instLsb = nil
            instPc = nil
            return createInstrumentChangeMessage(partNr, l, p)
        end
        local function instrumentChangeHandler(leafNode, response)
            if not IsIdForPart(leafNode.fullid, partNr) then
                return nil
            end
            if leafNode.node.id == "SNTC_INST_BS_LSB" then
                instLsb = Bytes_To_Value(response.payload)
                return handleInstrumentChange(instLsb, instPc)
            end
            if leafNode.node.id == "SNTC_INST_BS_PC" then
                instPc = Bytes_To_Value(response.payload)
                return handleInstrumentChange(instLsb, instPc)
            end
            return nil
        end
        AddReceiveHandler(instrumentChangeHandler)
        snaData.getReceiveValueSysex = function ()
            return CreateReceiveMessageForBranch("PRM-_FPART".. partNr .."-_SNTONE")
        end
        snaData.name = name
        for _, subSection in ipairs(snaData.grp) do
            for _, param in ipairs(subSection.params) do
                local tmplId = param.id
                local isInstNr = param.id == idInstNr
                param.id = CreateId(param.id, partNr)
                if not isInstNr then
                    param = ParameterSetValueWrapper(param)
                else
                    param.setValue = function (value)
                        return instrumentChange(partNr, value)
                    end
                end
                for modNr = 1, 32, 1 do
                    if tmplId == idTmpl("MOD_PRM"..tostring(modNr)) then
                        param.name = function ()
                            return modName(partNr, modNr)
                        end
                        param.min = function ()
                            return modMin(partNr, modNr);
                        end
                        param.max = function ()
                            return modMax(partNr, modNr);
                        end
                        if kDrawbarColors[modNr] ~= nil then
                            local color = kDrawbarColors[modNr]
                            param.drawbarColor = function()
                                local instrData = getInstrumentData(partNr)
                                if instrData ~= nil and instrData.modData == MOD_TW then
                                    return color
                                end
                                return ""
                            end
                        end
                    end
                end

            end
        end
        main[k] = snaData
    end
end
