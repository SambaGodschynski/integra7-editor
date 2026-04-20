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
local msbToTypeIdx = {[89]=1, [95]=2, [88]=3, [87]=4, [86]=5}

-- Update the NEFP_TYPE_DUMMY display param for a part based on a known MSB value.
-- Called from preset browser (immediate) and from BuildSyncToneTypeRequest (after device read).
function SyncToneTypeFromMsb(partNr, msb)
    local typeIdx = msbToTypeIdx[msb]
    if typeIdx then
        UpdateParamDisplay("PRM-_PRF-_FP"..partNr.."-NEFP_TYPE_DUMMY", typeIdx)
    end
end

-- Returns a RequestMessage that reads NEFP_PAT_BS_MSB from the device and syncs NEFP_TYPE_DUMMY.
-- Used by Sidebar when a Part accordion is opened.
function BuildSyncToneTypeRequest(partNr)
    local msbId = "PRM-_PRF-_FP"..partNr.."-NEFP_PAT_BS_MSB"
    local msg = CreateReceiveMessageForLeafId(msbId)
    if msg == nil then return nil end
    local origHandler = msg.onMessageReceived
    msg.onMessageReceived = function(bytes)
        local result = origHandler(bytes)
        for _, vcm in ipairs(result) do
            if vcm.id == msbId then
                SyncToneTypeFromMsb(partNr, vcm.i7Value)
            end
        end
        return result
    end
    return msg
end

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
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-"
    local s = {
        name = "Part " .. pn .. " Common",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            for _, suffix in ipairs({
                "NEFP_LEVEL","NEFP_PAN","NEFP_CHO_SEND","NEFP_REV_SEND",
                "NEFP_MUTE_SW","NEFP_RX_SW","NEFP_RX_CH","NEFP_OUT_ASGN",
                "NEFP_MONO_POLY","NEFP_LEGATO_SW",
            }) do
                local msg = CreateReceiveMessageForLeafId(fp..suffix)
                if msg then table.insert(result, msg) end
            end
            local msg = CreateReceiveMessageForLeafId("PRM-_PRF-_FC-NEFC_VOICE_RESERV"..i)
            if msg then table.insert(result, msg) end
            return result
        end,
    }
    if i == 1 then
        table.insert(s.params, p({type="range", id=SOLO_PARAM_ID, name=get("__HIDDEN__"), min=get(0), max=get(16), default=0}))
    end
    table.insert(s.params, p({type="range",  id=fp.."NEFP_LEVEL",        name=get(partName.." Level"),        min=get(0),   max=get(127), default=100}))
    table.insert(s.params, p({type="range",  id=fp.."NEFP_PAN",          name=get(partName.." Pan"),          min=get(-64), max=get(63),  default=0,  toI7Value=panToI7, toGuiValue=panToGui}))
    table.insert(s.params, p({type="range",  id=fp.."NEFP_CHO_SEND",     name=get(partName.." Chorus"),       min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."NEFP_REV_SEND",     name=get(partName.." Reverb"),       min=get(0),   max=get(127), default=20}))
    table.insert(s.params, p({type="toggle", id=fp.."NEFP_MUTE_SW",      name=get(partName.." Mute"),         min=get(0),   max=get(1)}))
    table.insert(s.params,  {type="solotoggle", id="SOLO_TOGGLE_PART_"..i, name=get(partName.." Solo"),       linkedParamId=SOLO_PARAM_ID, linkedValue=i})
    table.insert(s.params, p({type="toggle", id=fp.."NEFP_RX_SW",        name=get(partName.." RX SW"),        min=get(0),   max=get(1),   default=1}))
    table.insert(s.params, p({type="select", id=fp.."NEFP_RX_CH",        name=get(partName.." RX CH"),        min=get(0),   max=get(15),  default=0,  options=rxChOptions}))
    table.insert(s.params, p({type="select", id=fp.."NEFP_OUT_ASGN",     name=get(partName.." Output"),       min=get(0),   max=get(11),  default=0,  options=outAssignOptions}))
    table.insert(s.params, p({type="select", id=fp.."NEFP_MONO_POLY",    name=get(partName.." Mono/Poly"),    min=get(0),   max=get(2),   default=2,  options=monoPolyOptions}))
    table.insert(s.params, p({type="select", id=fp.."NEFP_LEGATO_SW",    name=get(partName.." Legato"),       min=get(0),   max=get(2),   default=2,  options=legatoOptions}))
    table.insert(s.params, p({type="select", id="PRM-_PRF-_FC-NEFC_VOICE_RESERV"..i, name=get(partName.." Voice Reserve"), min=get(0), max=get(64), default=0, options=voiceReservOptions}))
    table.insert(s.params,  {type="select",  id=fp.."NEFP_TYPE_DUMMY",   name=get(partName.." Type"),         default=1,    options=toneTypes, setValue=partTypeChange(i)})
    table.insert(s.params,  {type="savesysex", id="SAVE_SYSEX_PART_"..i, name=get("Save SysEx Part "..pn),   partPrefix="Part " .. pn .. " "})
    return s
end

-- Scale section ----------------------------------------------------------

local scaleTbl = {
    [1] = { 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0 },
    [2] = { 16, -14,  20,  31,   2,  14, -16,  18, -12,   0,  33,   4 },
    [3] = { 16,  49,  20,  31,   2,  14,  47,  18,  29,   0,  33,   4 },
    [4] = { -6,   8,  -2, -12,   2,  -8,   6,  -4,  10,   0, -10,   4 },
    [5] = { 10,   0,   3,   4,  -3,   8,   0,   7,   2,   0,   6,  -1 },
    [6] = { 10, -14,   3,  21,  -3,  14, -10,   7, -17,   0,  17,  -7 },
    [7] = { 12,   2,   4,   6,   2,  10,   0,   8,   4,   0,   8,   4 },
    [8] = { -6,  45,  -2, -12,  51,  -8,  43,  -4,  47,   0, -10, -49 },
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

local scaleTypeState = {}
local scaleKeyState  = {}
local tuneState      = {}
for i = 1, 16 do
    scaleTypeState[i] = 1
    scaleKeyState[i]  = 0
    tuneState[i]      = {0,0,0,0,0,0,0,0,0,0,0,0}
end

local function computeTemperament(scaleType, key)
    local tbl   = scaleTbl[scaleType]
    local shift = (9 - key >= 0) and (9 - key) or (9 - key + 11)
    local ofst  = tbl[9 + 1] - tbl[shift + 1]
    local result = {}
    if scaleType == 8 then
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

local function makeScaleSection(i)
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local fp       = "PRM-_PRF-_FP"..i.."-NEFP_"
    local scaleSuffixes = {
        "NEFP_SCALE_TYPE","NEFP_SCALE_KEY",
        "NEFP_TUNE_C","NEFP_TUNE_CS","NEFP_TUNE_D","NEFP_TUNE_DS",
        "NEFP_TUNE_E","NEFP_TUNE_F","NEFP_TUNE_FS","NEFP_TUNE_G",
        "NEFP_TUNE_GS","NEFP_TUNE_A","NEFP_TUNE_AS","NEFP_TUNE_B",
    }
    local s = {
        name = "Part " .. pn .. " Scale",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            local fpBase = "PRM-_PRF-_FP"..i.."-"
            for _, suffix in ipairs(scaleSuffixes) do
                local msg = CreateReceiveMessageForLeafId(fpBase..suffix)
                if msg then table.insert(result, msg) end
            end
            return result
        end,
    }

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
        scaleTypeState[i] = t
        if t ~= 0 then
            tuneState[i] = computeTemperament(t, scaleKeyState[i])
        end
    end)
    table.insert(s.params, typeParam)

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
        scaleKeyState[i] = k
        if scaleTypeState[i] ~= 0 then
            tuneState[i] = computeTemperament(scaleTypeState[i], k)
        end
    end)
    table.insert(s.params, keyParam)

    for noteIdx = 1, 12 do
        local noteName = noteNames[noteIdx]
        local tuneId   = fp.."TUNE_"..noteName
        local nIdx     = noteIdx
        local tuneParam = {
            type          = "range",
            id            = tuneId,
            name          = get(partName.." "..noteDisplay[noteIdx]),
            min           = get(-64),
            max           = get(63),
            default       = 0,
            toI7Value     = function(gui) return math.tointeger(64 + gui) end,
            toGuiValue    = function(i7)  return math.tointeger(i7 - 64) end,
            valueOverride = function() return tuneState[i][nIdx] end,
        }
        p(tuneParam, function(i7val)
            tuneState[i][nIdx] = math.tointeger(i7val - 64)
        end)
        table.insert(s.params, tuneParam)
    end

    return s
end

-- MIDI section ----------------------------------------------------------

local function makeMidiSection(i)
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local midiSuffixes = {
        "NEFP_RX_PC","NEFP_RX_BS","NEFP_RX_BEND","NEFP_RX_PAFT",
        "NEFP_RX_CAFT","NEFP_RX_MOD","NEFP_RX_VOL","NEFP_RX_PAN",
        "NEFP_RX_EXPR","NEFP_RX_HOLD","NEFP_VELO_CRV_TYPE",
    }
    local s = {
        name = "Part " .. pn .. " MIDI",
        layout = "inline_toggles",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            local fpBase = "PRM-_PRF-_FP"..i.."-"
            for _, suffix in ipairs(midiSuffixes) do
                local msg = CreateReceiveMessageForLeafId(fpBase..suffix)
                if msg then table.insert(result, msg) end
            end
            return result
        end,
    }
    table.insert(s.params, p({type="toggle", id=fp.."RX_PC",         name=get(partName.." RX Prog Chg"),   min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_BS",         name=get(partName.." RX Bank Sel"),   min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_BEND",       name=get(partName.." RX Pitch Bend"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_PAFT",       name=get(partName.." RX Poly AfTch"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_CAFT",       name=get(partName.." RX Chan AfTch"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_MOD",        name=get(partName.." RX Modulation"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_VOL",        name=get(partName.." RX Volume"),     min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_PAN",        name=get(partName.." RX Pan"),        min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_EXPR",       name=get(partName.." RX Expression"), min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="toggle", id=fp.."RX_HOLD",       name=get(partName.." RX Hold-1"),     min=get(0), max=get(1), default=1}))
    table.insert(s.params, p({type="range",  id=fp.."VELO_CRV_TYPE", name=get(partName.." Velo Crv"),      min=get(0), max=get(4), default=0}))
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
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local s = {
        name = "Part " .. pn .. " Offset",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            local fpBase = "PRM-_PRF-_FP"..i.."-"
            for _, suffix in ipairs({
                "NEFP_CUTOFF_OFST","NEFP_RESO_OFST","NEFP_ATK_OFST",
                "NEFP_DCY_OFST","NEFP_REL_OFST",
                "NEFP_VIB_RATE","NEFP_VIB_DEPTH","NEFP_VIB_DELAY",
            }) do
                local msg = CreateReceiveMessageForLeafId(fpBase..suffix)
                if msg then table.insert(result, msg) end
            end
            return result
        end,
    }
    table.insert(s.params, p({type="range", id=fp.."CUTOFF_OFST", name=get(partName.." Cutoff Ofs"),  min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."RESO_OFST",   name=get(partName.." Reso Ofs"),    min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."ATK_OFST",    name=get(partName.." Attack Ofs"),  min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."DCY_OFST",    name=get(partName.." Decay Ofs"),   min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."REL_OFST",    name=get(partName.." Release Ofs"), min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."VIB_RATE",    name=get(partName.." Vib Rate"),    min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."VIB_DEPTH",   name=get(partName.." Vib Depth"),   min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range", id=fp.."VIB_DELAY",   name=get(partName.." Vib Delay"),   min=get(-64), max=get(63), default=0, toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    return s
end

local function makePitchSection(i)
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local s = {
        name = "Part " .. pn .. " Pitch",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            local fpBase = "PRM-_PRF-_FP"..i.."-"
            for _, suffix in ipairs({
                "NEFP_OCTAVE","NEFP_PIT_CRS","NEFP_PIT_FINE",
                "NEFP_BEND_RANGE","NEFP_PORT_SW","NEFP_PORT_TIME",
            }) do
                local msg = CreateReceiveMessageForLeafId(fpBase..suffix)
                if msg then table.insert(result, msg) end
            end
            return result
        end,
    }
    table.insert(s.params, p({type="range",  id=fp.."OCTAVE",     name=get(partName.." Octave Shift"), min=get(-3),  max=get(3),   default=0,   toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range",  id=fp.."PIT_CRS",    name=get(partName.." Coarse Tune"),  min=get(-48), max=get(48),  default=0,   toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range",  id=fp.."PIT_FINE",   name=get(partName.." Fine Tune"),    min=get(-50), max=get(50),  default=0,   toI7Value=offsetToI7, toGuiValue=offsetToGui}))
    table.insert(s.params, p({type="range",  id=fp.."BEND_RANGE", name=get(partName.." Bend Range"),   min=get(0),   max=get(25),  default=25}))
    table.insert(s.params, p({type="select", id=fp.."PORT_SW",    name=get(partName.." Porta SW"),     min=get(0),   max=get(2),   default=2,   options=portSwOptions}))
    table.insert(s.params, p({type="select", id=fp.."PORT_TIME",  name=get(partName.." Porta Time"),   min=get(0),   max=get(128), default=128, options=portTimeOptions}))
    return s
end

-- Keyboard section ----------------------------------------------------------
local function vSensToI7(gui)  return math.tointeger(64 + gui) end
local function vSensToGui(i7)  return math.tointeger(i7 - 64) end

local veloCrvOptions = {[0]="OFF",[1]="1",[2]="2",[3]="3",[4]="4"}

local function makeKeyboardSection(i)
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local fp = "PRM-_PRF-_FP"..i.."-NEFP_"
    local s = {
        name = "Part " .. pn .. " Keyboard",
        layout = "keyboard",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            local fpBase = "PRM-_PRF-_FP"..i.."-"
            for _, suffix in ipairs({
                "NEFP_KFADE_LO","NEFP_KRANGE_LO","NEFP_KRANGE_UP","NEFP_KFADE_UP",
                "NEFP_VFADE_LO","NEFP_VRANGE_LO","NEFP_VRANGE_UP","NEFP_VFADE_UP",
                "NEFP_VSENS_OFST","NEFP_VELO_CRV_TYPE",
            }) do
                local msg = CreateReceiveMessageForLeafId(fpBase..suffix)
                if msg then table.insert(result, msg) end
            end
            return result
        end,
    }
    table.insert(s.params, p({type="range",  id=fp.."KFADE_LO",      name=get(partName.." Key Fade Lo"),  min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."KRANGE_LO",     name=get(partName.." Key Range Lo"), min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."KRANGE_UP",     name=get(partName.." Key Range Hi"), min=get(0),   max=get(127), default=127}))
    table.insert(s.params, p({type="range",  id=fp.."KFADE_UP",      name=get(partName.." Key Fade Hi"),  min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."VFADE_LO",      name=get(partName.." Vel Fade Lo"),  min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."VRANGE_LO",     name=get(partName.." Vel Range Lo"), min=get(1),   max=get(127), default=1}))
    table.insert(s.params, p({type="range",  id=fp.."VRANGE_UP",     name=get(partName.." Vel Range Hi"), min=get(1),   max=get(127), default=127}))
    table.insert(s.params, p({type="range",  id=fp.."VFADE_UP",      name=get(partName.." Vel Fade Hi"),  min=get(0),   max=get(127), default=0}))
    table.insert(s.params, p({type="range",  id=fp.."VSENS_OFST",    name=get(partName.." Vel Sens Ofs"), min=get(-63), max=get(63),  default=0,  toI7Value=vSensToI7, toGuiValue=vSensToGui}))
    table.insert(s.params, p({type="select", id=fp.."VELO_CRV_TYPE", name=get(partName.." Velo Crv"),     min=get(0),   max=get(4),   default=0,  options=veloCrvOptions}))
    return s
end

local function makeEqSection(i)
    local pn = string.format("%02d", i)
    local partName = "Part " .. i
    local prefix = "PRM-_PRF-_FPEQ"..i.."-NEFPEQ_EQ_"
    local s = {
        name = "Part " .. pn .. " EQ",
        layout = "eq3band",
        params = {},
        getReceiveValueSysex = function()
            return CreateReceiveMessageForBranch("PRM-_PRF-_FPEQ"..i)
        end,
    }
    table.insert(s.params, p({type="toggle",  id=prefix.."SW",       name=get(partName.." EQ SW"),     min=get(0),   max=get(1),  default=0}))
    table.insert(s.params, p({type="vslider", id=prefix.."LOWGAIN",  name=get(partName.." Low Gain"),  min=get(-15), max=get(15), default=0, toI7Value=eqGainToI7, toGuiValue=eqGainToGui}))
    table.insert(s.params, p({type="vslider", id=prefix.."MIDGAIN",  name=get(partName.." Mid Gain"),  min=get(-15), max=get(15), default=0, toI7Value=eqGainToI7, toGuiValue=eqGainToGui}))
    table.insert(s.params, p({type="vslider", id=prefix.."HIGHGAIN", name=get(partName.." High Gain"), min=get(-15), max=get(15), default=0, toI7Value=eqGainToI7, toGuiValue=eqGainToGui}))
    table.insert(s.params, p({type="range",   id=prefix.."LOWFREQ",  name=get(partName.." Low Freq"),  min=get(0),   max=get(1),  default=1}))
    table.insert(s.params, p({type="range",   id=prefix.."MIDFREQ",  name=get(partName.." Mid Freq"),  min=get(0),   max=get(16), default=7}))
    table.insert(s.params, p({type="range",   id=prefix.."MIDQ",     name=get(partName.." Mid Q"),     min=get(0),   max=get(4),  default=0}))
    table.insert(s.params, p({type="range",   id=prefix.."HIGHFREQ", name=get(partName.." High Freq"), min=get(0),   max=get(2),  default=1}))
    return s
end

function CreatePartsSections(main)
    for i = 1, 16, 1 do
        local pn = string.format("%02d", i)
        main["Part " .. pn .. " Common"]   = makeCommonSection(i)
        main["Part " .. pn .. " EQ"]       = makeEqSection(i)
        main["Part " .. pn .. " Keyboard"] = makeKeyboardSection(i)
        main["Part " .. pn .. " Pitch"]    = makePitchSection(i)
        main["Part " .. pn .. " Offset"]   = makeOffsetSection(i)
        main["Part " .. pn .. " Scale"]    = makeScaleSection(i)
        main["Part " .. pn .. " MIDI"]     = makeMidiSection(i)
    end
end
