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

-- Scale section ----------------------------------------------------------

-- Preset tuning tables (GUI values, -64..+63) indexed by SCALE_TYPE (1-8).
-- Row order: C, C#, D, D#, E, F, F#, G, G#, A, A#, B  (0-based note index)
local scaleTbl = {
    [1] = { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 }, -- EQUAL
    [2] = { 16, -14,  20,  31,   2,  14, -16,  18, -12,   0,  33,   4 }, -- JUST-MAJ
    [3] = { 16,  49,  20,  31,   2,  14,  47,  18,  29,   0,  33,   4 }, -- JUST-MIN
    [4] = { -6,   8,  -2, -12,   2,  -8,   6,  -4,  10,   0, -10,   4 }, -- PYTHAGORE
    [5] = { 10,   0,   3,   4,  -3,   8,   0,   7,   2,   0,   6,  -1 }, -- KIRNBERGE
    [6] = { 10, -14,   3,  21,  -3,  14, -10,   7, -17,   0,  17,  -7 }, -- MEANTONE
    [7] = { 12,   2,   4,   6,   2,  10,   0,   8,   4,   0,   8,   4 }, -- WERCKMEIS
    [8] = { -6,  45,  -2, -12,  51,  -8,  43,  -4,  47,   0, -10, -49 }, -- ARABIC
}

local scaleTypeOptions = {
    [0]="CUSTOM",[1]="EQUAL",[2]="JUST-MAJ",[3]="JUST-MIN",[4]="PYTHAGORE",
    [5]="KIRNBERGE",[6]="MEANTONE",[7]="WERCKMEIS",[8]="ARABIC"
}
local scaleKeyOptions = {
    [0]="C",[1]="C#",[2]="D",[3]="D#",[4]="E",[5]="F",
    [6]="F#",[7]="G",[8]="G#",[9]="A",[10]="A#",[11]="B"
}
local noteNames    = {"C","CS","D","DS","E","F","FS","G","GS","A","AS","B"}
local noteDisplay  = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"}

-- Per-part runtime state
local scaleTypeState = {}
local scaleKeyState  = {}
local tuneState      = {} -- [part][1..12] = gui value (-64..+63)
for i = 1, 16 do
    scaleTypeState[i] = 1  -- EQUAL
    scaleKeyState[i]  = 0  -- C
    tuneState[i]      = {0,0,0,0,0,0,0,0,0,0,0,0}
end

-- Compute the 12 note gui-values for a given preset type and key.
-- type: 1-8 (matches scaleTbl), key: 0-11
local function computeTemperament(scaleType, key)
    local tbl   = scaleTbl[scaleType]
    local shift = (9 - key >= 0) and (9 - key) or (9 - key + 11)
    local ofst  = tbl[9 + 1] - tbl[shift + 1]  -- 1-based Lua indexing
    local result = {}
    if scaleType == 8 then  -- ARABIC: rotate by key, no offset
        for i = 0, 11 do
            local v = (i - key >= 0) and (i - key) or (i - key + 12)
            result[i + 1] = tbl[v + 1]
        end
    else
        for i = 0, 11 do
            local idx = (i + key < 12) and (i + key) or (i + key - 12)
            result[idx + 1] = tbl[i + 1] + ofst
        end
    end
    return result
end

local function makeScaleSection(partNr)
    local partName = "Part " .. partNr
    local fp       = "PRM-_PRF-_FP"..partNr.."-NEFP_"
    local s        = { name = "Part " .. partNr .. " Scale", params = {} }

    -- SCALE_TYPE: send only TYPE sysex; update tuneState (no TUNE sysex)
    local typeId = fp.."SCALE_TYPE"
    local typeParam = {
        type    = "select",
        id      = typeId,
        name    = get(partName.." Scale Type"),
        min     = get(0),
        max     = get(8),
        default = 1,
        options = scaleTypeOptions,
    }
    p(typeParam, function(i7val)
        local t = math.tointeger(i7val)
        scaleTypeState[partNr] = t
        if t ~= 0 then
            tuneState[partNr] = computeTemperament(t, scaleKeyState[partNr])
        end
    end)
    table.insert(s.params, typeParam)

    -- SCALE_KEY: send only KEY sysex; recompute tuneState if type != CUSTOM
    local keyId = fp.."SCALE_KEY"
    local keyParam = {
        type    = "select",
        id      = keyId,
        name    = get(partName.." Scale Key"),
        min     = get(0),
        max     = get(11),
        default = 0,
        options = scaleKeyOptions,
    }
    p(keyParam, function(i7val)
        local k = math.tointeger(i7val)
        scaleKeyState[partNr] = k
        if scaleTypeState[partNr] ~= 0 then
            tuneState[partNr] = computeTemperament(scaleTypeState[partNr], k)
        end
    end)
    table.insert(s.params, keyParam)

    -- 12 TUNE params: valueOverride reads tuneState (updated by preset selection).
    -- setValue sends individual sysex and updates tuneState.
    for noteIdx = 1, 12 do
        local noteName = noteNames[noteIdx]
        local tuneId   = fp.."TUNE_"..noteName
        local nIdx     = noteIdx  -- capture
        local tuneParam = {
            type          = "range",
            id            = tuneId,
            name          = get(partName.." "..noteDisplay[noteIdx]),
            min           = get(-64),
            max           = get(63),
            default       = 0,
            toI7Value     = function(gui) return math.tointeger(64 + gui) end,
            toGuiValue    = function(i7)  return math.tointeger(i7 - 64) end,
            valueOverride = function() return tuneState[partNr][nIdx] end,
        }
        p(tuneParam, function(i7val)
            tuneState[partNr][nIdx] = math.tointeger(i7val - 64)
        end)
        table.insert(s.params, tuneParam)
    end

    return s
end

-- MIDI section ----------------------------------------------------------

local function makeMidiSection(i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local s = { name = "Part " .. i .. " MIDI", layout = "inline_toggles", params = {} }
    table.insert(s.params, p({type="toggle", id=fp.."RX_PC",   name=get(partName.." RX Prog Chg"),  min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_BS",   name=get(partName.." RX Bank Sel"),  min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_BEND", name=get(partName.." RX Pitch Bend"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_PAFT", name=get(partName.." RX Poly AfTch"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_CAFT", name=get(partName.." RX Chan AfTch"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_MOD",  name=get(partName.." RX Modulation"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_VOL",  name=get(partName.." RX Volume"),     min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_PAN",  name=get(partName.." RX Pan"),        min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_EXPR", name=get(partName.." RX Expression"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_HOLD", name=get(partName.." RX Hold-1"),     min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="range",  id=fp.."VELO_CRV_TYPE", name=get(partName.." Velo Crv"), min=get(0), max=get(4), default=0}))
    return s
end

-- Pitch section ----------------------------------------------------------
local function offsetToI7(gui) return math.tointeger(64 + gui) end
local function offsetToGui(i7) return math.tointeger(i7 - 64) end

local portSwOptions = {[0]="OFF",[1]="ON",[2]="TONE"}

local portTimeOptions = {}
for v = 0, 127 do portTimeOptions[v] = tostring(v) end
portTimeOptions[128] = "TONE"

local function makeOffsetSection(i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local s = { name = "Part " .. i .. " Offset", params = {} }
    table.insert(s.params, p({type="range", id=fp.."CUTOFF_OFST", name=get(partName.." Cutoff Ofs"),   min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."RESO_OFST",   name=get(partName.." Reso Ofs"),     min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."ATK_OFST",    name=get(partName.." Attack Ofs"),   min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."DCY_OFST",    name=get(partName.." Decay Ofs"),    min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."REL_OFST",    name=get(partName.." Release Ofs"),  min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."VIB_RATE",    name=get(partName.." Vib Rate"),     min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."VIB_DEPTH",   name=get(partName.." Vib Depth"),    min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."VIB_DELAY",   name=get(partName.." Vib Delay"),    min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    return s
end

local function makePitchSection(i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local s = { name = "Part " .. i .. " Pitch", params = {} }
    table.insert(s.params, p({type="range",  id=fp.."OCTAVE",     name=get(partName.." Octave Shift"),  min=get(-3),  max=get(3),   default=0,  toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range",  id=fp.."PIT_CRS",    name=get(partName.." Coarse Tune"),   min=get(-48), max=get(48),  default=0,  toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range",  id=fp.."PIT_FINE",   name=get(partName.." Fine Tune"),     min=get(-50), max=get(50),  default=0,  toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range",  id=fp.."BEND_RANGE", name=get(partName.." Bend Range"),    min=get(0),   max=get(25),  default=25}))
    table.insert(s.params, p({type="select", id=fp.."PORT_SW",    name=get(partName.." Porta SW"),      min=get(0),   max=get(2),   default=2,  options=portSwOptions}))
    table.insert(s.params, p({type="select", id=fp.."PORT_TIME",  name=get(partName.." Porta Time"),    min=get(0),   max=get(128), default=128, options=portTimeOptions}))
    return s
end

-- Keyboard section: offset mapping for Velo Sens Offset
local function vSensToI7(gui)  return math.tointeger(64 + gui) end
local function vSensToGui(i7)  return math.tointeger(i7 - 64) end

-- Options for Velocity Curve Type: 0=OFF, 1-4 = curve 1..4
local veloCrvOptions = {[0]="OFF",[1]="1",[2]="2",[3]="3",[4]="4"}

local function makeKeyboardSection(i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    -- param order must match renderKeyboard:
    -- KFADE_LO, KRANGE_LO, KRANGE_UP, KFADE_UP,
    -- VFADE_LO, VRANGE_LO, VRANGE_UP, VFADE_UP, VSENS_OFST,
    -- VELO_CRV_TYPE
    local s = { name = "Part " .. i .. " Keyboard", layout = "keyboard", params = {} }
    table.insert(s.params, p({type="range",  id=fp.."KFADE_LO",      name=get(partName.." Key Fade Lo"),   min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."KRANGE_LO",     name=get(partName.." Key Range Lo"),  min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."KRANGE_UP",     name=get(partName.." Key Range Hi"),  min=get(0),   max=get(127), default=127}))
    table.insert(s.params, p({type="range",  id=fp.."KFADE_UP",      name=get(partName.." Key Fade Hi"),   min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."VFADE_LO",      name=get(partName.." Vel Fade Lo"),   min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."VRANGE_LO",     name=get(partName.." Vel Range Lo"),  min=get(1),   max=get(127), default=1}))
    table.insert(s.params, p({type="range",  id=fp.."VRANGE_UP",     name=get(partName.." Vel Range Hi"),  min=get(1),   max=get(127), default=127}))
    table.insert(s.params, p({type="range",  id=fp.."VFADE_UP",      name=get(partName.." Vel Fade Hi"),   min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."VSENS_OFST",    name=get(partName.." Vel Sens Ofs"),  min=get(-63), max=get(63),  default=0,  toI7Value=vSensToI7, toGuiValue=vSensToGui}))
    table.insert(s.params, p({type="select", id=fp.."VELO_CRV_TYPE", name=get(partName.." Velo Crv"),      min=get(0),   max=get(4),   default=0,  options=veloCrvOptions}))
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
        table.insert(parts.grp, makeKeyboardSection(i))
        table.insert(parts.grp, makePitchSection(i))
        table.insert(parts.grp, makeOffsetSection(i))
        table.insert(parts.grp, makeScaleSection(i))
        table.insert(parts.grp, makeMidiSection(i))
    end
    main["parts"] = parts
end
