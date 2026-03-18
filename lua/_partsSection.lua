require "_com"
require "_sysex"

local p = ParameterSetValueWrapper;
local get = GetWrapper
local SOLO_PARAM_ID = "PRM-_PRF-_FC-NEFC_SOLO_PART"
local toneTypes = {"SN-A", "SN-S", "SN-D", "PCM-S", "PCM-D"}
local toneTypeValues = {
    { msb=89, lsb=64 },
    { msb=95, lsb=64 },
    { msb=88, lsb=64 },
    { msb=87, lsb=64 },
    { msb=86, lsb=64 },
}

local function partTypeChange(part)
    return function (value)
        local values = toneTypeValues[value]
        local msbId = "PRM-_PRF-_FP" .. tostring(part) .. "-NEFP_PAT_BS_MSB"
        local msbMessage = CreateSysexMessage(msbId, values.msb)

        local lsbId = "PRM-_PRF-_FP" .. tostring(part) .. "-NEFP_PAT_BS_LSB"
        local lsbMessage = CreateSysexMessage(lsbId, values.lsb)

        local pcId = "PRM-_PRF-_FP" .. tostring(part) .. "-NEFP_PAT_PC"
        local pcMessage = CreateSysexMessage(pcId, 0)

        local msg = Concat(msbMessage, lsbMessage)
        msg = Concat(msg, pcMessage)
        return msg
    end
end

-- Collect all RequestMessages for a given part by iterating Main at call time.
-- Uses the "Part NN " prefix to find all matching sections with getReceiveValueSysex.
local function buildReceiveAllAction(partNr)
    local prefix = "Part " .. string.format("%02d", partNr) .. " "
    return function()
        local result = {}
        for key, section in pairs(Main) do
            if string.sub(key, 1, #prefix) == prefix and section.getReceiveValueSysex then
                for _, msg in ipairs(section.getReceiveValueSysex()) do
                    table.insert(result, msg)
                end
            end
        end
        return result
    end
end

-- Common params
local function panToI7(gui) return math.tointeger(64 + gui) end
local function panToGui(i7) return math.tointeger(i7 - 64) end

local outAssignOptions = {[0]="A",[1]="B",[2]="C",[3]="D",[4]="1",[5]="2",[6]="3",[7]="4",[8]="5",[9]="6",[10]="7",[11]="8"}
local rxChOptions = {}
for ch = 0, 15 do rxChOptions[ch] = tostring(ch + 1) end
local monoPolyOptions = {[0]="MONO",[1]="POLY",[2]="TONE"}
local legatoOptions   = {[0]="OFF",[1]="ON",[2]="TONE"}
local voiceReservOptions = {}
for v = 0, 63 do voiceReservOptions[v] = tostring(v) end
voiceReservOptions[64] = "FULL"

-- EQ params
local function eqGainToI7(gui)  return math.tointeger(math.floor(gui + 15 + 0.5)) end
local function eqGainToGui(i7)  return math.tointeger(math.floor(i7 - 15 + 0.5)) end

local eqLowFreqOptions  = {[0]="200",[1]="400"}
local eqMidFreqOptions  = {[0]="200",[1]="250",[2]="315",[3]="400",[4]="500",[5]="630",
                           [6]="800",[7]="1000",[8]="1250",[9]="1600",[10]="2000",
                           [11]="2500",[12]="3150",[13]="4000",[14]="5000",[15]="6300",[16]="8000"}
local eqMidQOptions     = {[0]="0.5",[1]="1.0",[2]="2.0",[3]="4.0",[4]="8.0"}
local eqHighFreqOptions = {[0]="2000",[1]="4000",[2]="8000"}

local function makeCommonSection(i)
    local partName = "Part " .. i
    local s = { name = "Part " .. i .. " Common", params = {} }
    table.insert(s.params, p({type="range",  id="PRM-_PRF-_FP"..i.."-NEFP_LEVEL",        name=get(partName.." Level"),        min=get(0),   max=get(127), default=100}))
    table.insert(s.params, p({type="range",  id="PRM-_PRF-_FP"..i.."-NEFP_PAN",          name=get(partName.." Pan"),          min=get(-64), max=get(63),  default=0,  toI7Value=panToI7, toGuiValue=panToGui}))
    table.insert(s.params, p({type="range",  id="PRM-_PRF-_FP"..i.."-NEFP_CHO_SEND",     name=get(partName.." Chorus"),       min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id="PRM-_PRF-_FP"..i.."-NEFP_REV_SEND",     name=get(partName.." Reverb"),       min=get(0),   max=get(127), default=20}))
    table.insert(s.params, p({type="toggle", id="PRM-_PRF-_FP"..i.."-NEFP_MUTE_SW",      name=get(partName.." Mute"),         min=get(0),   max=get(1)}))
    table.insert(s.params,  {type="solotoggle", id="SOLO_TOGGLE_PART_"..i,                name=get(partName.." Solo"),         linkedParamId=SOLO_PARAM_ID, linkedValue=i})
    table.insert(s.params, p({type="toggle", id="PRM-_PRF-_FP"..i.."-NEFP_RX_SW",        name=get(partName.." RX SW"),        min=get(0),   max=get(1),   default=1}))
    table.insert(s.params, p({type="select", id="PRM-_PRF-_FP"..i.."-NEFP_RX_CH",        name=get(partName.." RX CH"),        min=get(0),   max=get(15),  default=0,  options=rxChOptions}))
    table.insert(s.params, p({type="select", id="PRM-_PRF-_FP"..i.."-NEFP_OUT_ASGN",     name=get(partName.." Output"),       min=get(0),   max=get(11),  default=0,  options=outAssignOptions}))
    table.insert(s.params, p({type="select", id="PRM-_PRF-_FP"..i.."-NEFP_MONO_POLY",    name=get(partName.." Mono/Poly"),    min=get(0),   max=get(2),   default=2,  options=monoPolyOptions}))
    table.insert(s.params, p({type="select", id="PRM-_PRF-_FP"..i.."-NEFP_LEGATO_SW",    name=get(partName.." Legato"),       min=get(0),   max=get(2),   default=2,  options=legatoOptions}))
    table.insert(s.params, p({type="select", id="PRM-_PRF-_FC-NEFC_VOICE_RESERV"..i,     name=get(partName.." Voice Reserve"),min=get(0),   max=get(64),  default=0,  options=voiceReservOptions}))
    table.insert(s.params,  {type="select",  id="PRM-_PRF-_FP"..i.."-NEFP_TYPE_DUMMY",   name=get(partName.." Type"),         default=1,    options=toneTypes, setValue=partTypeChange(i)})
    table.insert(s.params,  {type="savesysex", id="SAVE_SYSEX_PART_"..i,                 name=get("Save SysEx Part "..string.format("%02d", i)),
                              partPrefix="Part " .. string.format("%02d", i) .. " "})
    return s
end

local function makeEqSection(i)
    local partName = "Part " .. i
    local prefix = "PRM-_PRF-_FPEQ"..i.."-NEFPEQ_EQ_"
    -- param order must match renderEq3Band: SW, LowGain, MidGain, HighGain, LowFreq, MidFreq, MidQ, HighFreq
    local s = { name = "Part " .. i .. " EQ", layout = "eq3band", params = {} }
    table.insert(s.params, p({type="toggle",  id=prefix.."SW",       name=get(partName.." EQ SW"),       min=get(0),   max=get(1),   default=0}))
    table.insert(s.params, p({type="vslider", id=prefix.."LOWGAIN",  name=get(partName.." Low Gain"),    min=get(-15), max=get(15),  default=0,  toI7Value=eqGainToI7, toGuiValue=eqGainToGui}))
    table.insert(s.params, p({type="vslider", id=prefix.."MIDGAIN",  name=get(partName.." Mid Gain"),    min=get(-15), max=get(15),  default=0,  toI7Value=eqGainToI7, toGuiValue=eqGainToGui}))
    table.insert(s.params, p({type="vslider", id=prefix.."HIGHGAIN", name=get(partName.." High Gain"),   min=get(-15), max=get(15),  default=0,  toI7Value=eqGainToI7, toGuiValue=eqGainToGui}))
    table.insert(s.params, p({type="range",   id=prefix.."LOWFREQ",  name=get(partName.." Low Freq"),    min=get(0),   max=get(1),   default=1}))
    table.insert(s.params, p({type="range",   id=prefix.."MIDFREQ",  name=get(partName.." Mid Freq"),    min=get(0),   max=get(16),  default=7}))
    table.insert(s.params, p({type="range",   id=prefix.."MIDQ",     name=get(partName.." Mid Q"),       min=get(0),   max=get(4),   default=0}))
    table.insert(s.params, p({type="range",   id=prefix.."HIGHFREQ", name=get(partName.." High Freq"),   min=get(0),   max=get(2),   default=1}))
    return s
end

function CreatePartsSections(main)
    local parts = {
        name = "Parts View",
        accordion = true,
        params = {
            {type="loadsysex", id="LOAD_SYSEX", name=get("Load SysEx")},
            p({type="range", id=SOLO_PARAM_ID, name=get("__HIDDEN__"), min=get(0), max=get(16), default=0})
        },
        grp = {}
    }
    for i = 1, 16, 1 do
        table.insert(parts.grp, makeCommonSection(i))
        table.insert(parts.grp, makeEqSection(i))
    end
    main["parts"] = parts
end
