require "math"
require "_sysex"
require "_com"
require "_model"
require "_snaData"  -- ToneCategories, MonoPoly

local get = GetWrapper

local function i7offsetValue(offset)
    return function(guiValue)
        return math.tointeger(offset + guiValue)
    end
end

local function guiOffsetValue(offset)
    return function(i7Value)
        return math.tointeger(i7Value - offset)
    end
end

local function idPc(name)
    return "PRM-_FPARTxxx-_PAT-_PC-RFPC_" .. name
end

local function idPc2(name)
    return "PRM-_FPARTxxx-_PAT-_PC2-RFPC2_" .. name
end

local function idPx(name)
    return "PRM-_FPARTxxx-_PAT-_PX-RFPX_" .. name
end

local Priority    = {[0]="Normal", [1]="High"}
local StretchTune = {[0]="Off", [1]="1", [2]="2", [3]="3"}
local PortMode    = {[0]="Normal", [1]="Legato"}
local PortType    = {[0]="Rate", [1]="Time"}
local PortStart   = {[0]="Pitch", [1]="Note"}
local StructureType = {[0]="1",[1]="2",[2]="3",[3]="4",[4]="5",[5]="6",[6]="7",[7]="8",[8]="9",[9]="10"}
local Booster       = {[0]="0dB",[1]="+6dB",[2]="+12dB",[3]="+18dB"}
local VeloCtrl      = {[0]="OFF",[1]="ON",[2]="RANDOM",[3]="CYCLE"}

local pcmsCommonTemplate = {
    name = "PCM-S Common",
    grp =
    {
        {
            name = "Tone",
            params =
            {
                {type="select", id=idPc2("CATE"),       name=get("Tone Category"),      default=0,   options=ToneCategories},
                {type="range",  id=idPc2("PHRASE_OCT"), name=get("Phrase Oct Shift"),   default=0,   min=get(-3),  max=get(3),   format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range",  id=idPc("LEVEL"),       name=get("Tone Level"),         default=127, min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=idPc("PAN"),         name=get("Tone Pan"),           default=0,   min=get(-64), max=get(63),  format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="select", id=idPc("PRIORITY"),    name=get("Tone Priority"),      default=0,   options=Priority},
                {type="range",  id=idPc("OCTAVE"),      name=get("Oct Shift"),          default=0,   min=get(-3),  max=get(3),   format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range",  id=idPc("PIT_CRS"),     name=get("Coarse Tune"),        default=0,   min=get(-48), max=get(48),  format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range",  id=idPc("PIT_FINE"),    name=get("Fine Tune"),          default=0,   min=get(-50), max=get(50),  format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="select", id=idPc("STRETCH"),     name=get("Stretch Tune Depth"), default=0,   options=StretchTune},
                {type="range",  id=idPc("ANALOG_FEEL"), name=get("Analog Feel"),        default=0,   min=get(0),   max=get(127), format="%.0f"},
            }
        },
        {
            name = "Filter / Amp Offsets",
            params =
            {
                {type="range", id=idPc("CUTOFF_OFST"), name=get("Cutoff Offset"),        default=0, min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range", id=idPc("RESO_OFST"),   name=get("Resonance Offset"),     default=0, min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range", id=idPc("ATK_OFST"),    name=get("Attack Time Offset"),   default=0, min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range", id=idPc("REL_OFST"),    name=get("Release Time Offset"),  default=0, min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
                {type="range", id=idPc("VSENS_OFST"),  name=get("Velocity Sens Offset"), default=0, min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offsetValue(64), toGuiValue=guiOffsetValue(64)},
            }
        },
        {
            name = "Portamento",
            params =
            {
                {type="select", id=idPc("MONO_POLY"),     name=get("Mono/Poly"),         default=1, options=MonoPoly},
                {type="toggle", id=idPc("LEGATO_SW"),     name=get("Legato Switch"),     default=0, min=get(0), max=get(1)},
                {type="toggle", id=idPc("LEGATO_RETRIG"), name=get("Legato Retrigger"),  default=0, min=get(0), max=get(1)},
                {type="toggle", id=idPc("PORT_SW"),       name=get("Portamento Switch"), default=0, min=get(0), max=get(1)},
                {type="select", id=idPc("PORT_MODE"),     name=get("Portamento Mode"),   default=0, options=PortMode},
                {type="select", id=idPc("PORT_TYPE"),     name=get("Portamento Type"),   default=0, options=PortType},
                {type="select", id=idPc("PORT_START"),    name=get("Portamento Start"),  default=0, options=PortStart},
                {type="range",  id=idPc("PORT_TIME"),     name=get("Portamento Time"),   default=20, min=get(0), max=get(127), format="%.0f"},
            }
        },
        {
            name = "Pitch Common",
            params =
            {
                {type="separator", id="PCMS_PITCH_SEP_xxx",       name=get("Pitch")},
                {type="range", id=idPc("BEND_RANGE_UP"), name=get("Pitch Bend Range Up"),   default=2, min=get(0), max=get(48), format="%.0f"},
                {type="range", id=idPc("BEND_RANGE_DW"), name=get("Pitch Bend Range Down"), default=2, min=get(0), max=get(48), format="%.0f"},
            }
        },
        {
            name = "PMT Common",
            params =
            {
                {type="separator", id="PCMS_PMT_SEP_xxx",          name=get("PMT")},
                {type="select", id=idPx("STRUCT1"),       name=get("Structure Type 1&2"),   default=0, options=StructureType},
                {type="select", id=idPx("STRUCT3"),       name=get("Structure Type 3&4"),   default=0, options=StructureType},
                {type="select", id=idPx("BOOST1"),        name=get("Booster 1&2"),          default=0, options=Booster},
                {type="select", id=idPx("BOOST3"),        name=get("Booster 3&4"),          default=0, options=Booster},
                {type="select", id=idPx("TMT_VELO_CTRL"), name=get("PMT Velocity Control"), default=1, options=VeloCtrl},
                {type="toggle", id=idPc("TMT_CTRL_SW"),   name=get("PMT Control Switch"),   default=0, min=get(0), max=get(1)},
            }
        },
    }
}

function CreatePcmsCommonSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " PCM-S Common"
        local sectionData = DeepCopy(pcmsCommonTemplate)
        sectionData.name = k

        sectionData.getReceiveValueSysex = function()
            local msgsPC  = CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PC")
            local msgsPC2 = CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PC2")
            local msgsPX  = CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PX")
            return Concat(Concat(msgsPC, msgsPC2), msgsPX)
        end

        for _, subSection in ipairs(sectionData.grp) do
            for _, param in ipairs(subSection.params) do
                param.id = CreateId(param.id, partNr)
                param = ParameterSetValueWrapper(param)
            end
        end

        main[k] = sectionData
    end
end
