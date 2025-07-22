require "math"
require "_snaData"
require "_sysex"
require "_com"

local idInstNr = "PRM-_FPARTxxx_InstNr"
local instrumentNumberPartMap = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}
local get = GetWrapper

local function i7offsetValue(offset)
    return function (guiValue)
        return math.tointeger(offset + guiValue)
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
    if instrumentData.modData[modNr] == nil then
        return 0
    end
    return instrumentData.modData[modNr].min
end

local function modMax(part, modNr)
    local instrumentData = getInstrumentData(part)
    if instrumentData == nil then
        return 127
    end
    if instrumentData.modData[modNr] == nil then
        return 127
    end
    return instrumentData.modData[modNr].max
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
    sub =
    {
        {
            name = "Common",
            params =
            {
              {type="select", id=idTmpl("CATE"), name=get("Tone Category"), default=0, options = ToneCategories},
              {type="range", id=idTmpl("PHRASE_OCT"), name=get("Phrase Oct Shift"), min=get(-3), max=get(3), default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("TONE_LEVEL"), name=get("Tone Level"), min=get(0), max=get(127), default=127, format="%.0f"},
              {type="select", id=idTmpl("MONO_POLY"), name=get("Mono/Poly"), default=1, options = MonoPoly},
              {type="range", id=idTmpl("OCTAVE"), name=get("Oct Shift"), min=get(-3), max=get(3), default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("PORT_TIME"), name=get("Portamento Time Offset"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("VIB_RATE"), name=get("Vibrato Rate"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("VIB_DEPTH"), name=get("Vibrato Depth"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
              {type="range", id=idTmpl("VIB_DELAY"), name=get("Vibrato Delay"), min=get(-64), max=get(63), default=0, format="%+.0f", toI7Value=i7offsetValue(64)},
            }
        },
        {
            name = "Instrument",
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

function CreateSnaSections(main)
    for partNr = 1, 16, 1 do
        local k = "SNA_" .. string.format("%02d", partNr)
        local name = k
        local snaData = DeepCopy(snaTemplate);
        snaData.name = name
        if partNr==1 then
            snaData.isOpen = true
        end
        for key, subSection in ipairs(snaData.sub) do
            for key, param in ipairs(subSection.params) do
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
                for modNr = 1, 22, 1 do
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
                    end
                end

            end
        end
        main[k] = snaData
    end
end
