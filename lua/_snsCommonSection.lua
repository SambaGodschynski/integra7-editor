require "_sysex"
require "_com"
require "_model"
require "_snaData"  -- ToneCategories

local get = GetWrapper

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end

local SyncRing   = {[0]="OFF", [1]="SYNC", [2]="RING"}
local PortMode   = {[0]="NORMAL", [1]="LEGATO"}
local UnisonSize = {[0]="2", [1]="4", [2]="6", [3]="8"}

function CreateSnsCommonSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " SN-S Common"

        local function idC(name)
            return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPC-SHPC_" .. name
        end

        local params = {
            {type="select", id=idC("CATE"),          name=get("Tone Category"),     default=0,   options=ToneCategories},
            {type="range",  id=idC("LEVEL"),         name=get("Tone Level"),        default=100, min=get(0),   max=get(127), format="%.0f"},
            {type="toggle", id=idC("PORT_SW"),       name=get("Portamento Switch"), default=0,   min=get(0),   max=get(1)},
            {type="range",  id=idC("PORT_TIME"),     name=get("Portamento Time"),   default=20,  min=get(0),   max=get(127), format="%.0f"},
            {type="select", id=idC("PORT_MODE"),     name=get("Portamento Mode"),   default=1,   options=PortMode},
            {type="toggle", id=idC("MONO_SW"),       name=get("Mono Switch"),       default=0,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("LEGATO_SW"),     name=get("Legato Switch"),     default=1,   min=get(0),   max=get(1)},
            {type="range",  id=idC("OCTAVE"),        name=get("Octave Shift"),      default=0,   min=get(-3),  max=get(3),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            {type="range",  id=idC("BEND_RANGE_UP"), name=get("Bend Range Up"),     default=2,   min=get(0),   max=get(24),  format="%.0f"},
            {type="range",  id=idC("BEND_RANGE_DW"), name=get("Bend Range Down"),   default=2,   min=get(0),   max=get(24),  format="%.0f"},
            {type="toggle", id=idC("TONE1_SW"),      name=get("Partial 1 Switch"),  default=0,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("TONE1_SEL"),     name=get("Partial 1 Select"),  default=0,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("TONE2_SW"),      name=get("Partial 2 Switch"),  default=0,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("TONE2_SEL"),     name=get("Partial 2 Select"),  default=0,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("TONE3_SW"),      name=get("Partial 3 Switch"),  default=0,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("TONE3_SEL"),     name=get("Partial 3 Select"),  default=0,   min=get(0),   max=get(1)},
            {type="select", id=idC("SYNC_RING_SEL"), name=get("Ring / Sync"),       default=0,   options=SyncRing},
            {type="toggle", id=idC("TFX_SW"),        name=get("TFX Switch"),        default=1,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("UNISON"),        name=get("Unison Switch"),     default=0,   min=get(0),   max=get(1)},
            {type="select", id=idC("UNISON_SIZE"),   name=get("Unison Size"),       default=3,   options=UnisonSize},
            {type="range",  id=idC("ANALOG_FEEL"),   name=get("Analog Feel"),       default=0,   min=get(0),   max=get(127), format="%.0f"},
            {type="range",  id=idC("CROSS_MOD"),     name=get("Wave Shape"),        default=0,   min=get(0),   max=get(127), format="%.0f"},
            {type="range",  id=idC("PHRASE"),        name=get("Phrase Number"),     default=0,   min=get(0),   max=get(65535), format="%.0f"},
            {type="range",  id=idC("PHRASE_OCT"),    name=get("Phrase Oct Shift"),  default=0,   min=get(-3),  max=get(3),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
        }

        for _, param in ipairs(params) do
            ParameterSetValueWrapper(param)
        end

        main[k] = {
            name = "Part " .. string.format("%02d", partNr) .. " SN-S Common",
            params = params,
            getReceiveValueSysex = function()
                return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPC")
            end,
        }
    end
end
