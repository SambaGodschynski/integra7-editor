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
            }
        }
    }
}

local function instrumentChange(part, index)
    instrumentNumberPartMap[part] = index
    local instrumentData = getInstrumentData(part)
    local lsb = instrumentData.lsb
    local pc = instrumentData.pc
    -- CreateSysexMessage(node_id, value)
    local lsbId = "PRM-_FPART" .. tostring(part) .. "-_SNTONE-_SNTC-SNTC_INST_BS_LSB"
    local lsbMessage = CreateSysexMessage(lsbId, lsb)
    local pcId = "PRM-_FPART" .. tostring(part) .. "-_SNTONE-_SNTC-SNTC_INST_BS_PC"
    local pcMessage = CreateSysexMessage(pcId, pc)
    local msg = Concat(lsbMessage, pcMessage)
    return msg
end

function CreateSnaSections(main)
    for i = 1, 16, 1 do
        local k = "SNA_" .. string.format("%02d", i)
        local name = k
        local snaData = DeepCopy(snaTemplate);
        snaData.name = name
        if i==1 then
            snaData.isOpen = true
        end
        for key, subSection in ipairs(snaData.sub) do
            for key, param in ipairs(subSection.params) do
                local isInstNr = param.id == idInstNr
                param.id = CreateId(param.id, i)
                if not isInstNr then
                    param = ParameterSetValueWrapper(param)
                else
                    param.setValue = function (value)
                        return instrumentChange(i, value)
                    end
                end
            end
        end
        main[k] = snaData
    end
end
