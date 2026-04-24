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

local EnvLoopMode = {[0]="OFF", [1]="FREE-RUN", [2]="TEMPO-SYNC"}
local EnvLoopNote = {
    [0]="1/16T", [1]="1/8T",  [2]="1/4T",  [3]="1/2T",
    [4]="1/16",  [5]="1/8",   [6]="3/16",  [7]="1/4",
    [8]="3/8",   [9]="1/2",   [10]="3/4",  [11]="1",
    [12]="3/2",  [13]="2",    [14]="3",     [15]="4",
    [16]="6",    [17]="8",    [18]="12",    [19]="16",
    [20]="24",   [21]="32",
}

function CreateSnsCommonSections(main)
    for partNr = 1, 16, 1 do
        local pn = string.format("%02d", partNr)

        local function idC(name)
            return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPC-SHPC_" .. name
        end
        local function idM(name)
            return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPTM-SHPTM_" .. name
        end

        -- Per-partial switch/select sections (referenced as tab header rows in Tone view)
        for i = 1, 3, 1 do
            local swKey = "Part " .. pn .. " SN-S Partial " .. i .. " Ctrl"
            local swParams = {
                {type="toggle", id=idC("TONE" .. i .. "_SW"),  name=get("Partial " .. i .. " Switch"), default=0, min=get(0), max=get(1)},
                {type="toggle", id=idC("TONE" .. i .. "_SEL"), name=get("Partial " .. i .. " Select"), default=0, min=get(0), max=get(1)},
            }
            for _, param in ipairs(swParams) do
                ParameterSetValueWrapper(param)
            end
            main[swKey] = {
                name   = "Part " .. string.format("%02d", partNr) .. " SN-S Partial " .. i .. " Ctrl",
                params = swParams,
                layout = "inline_toggles",
                getReceiveValueSysex = function()
                    local msgs = {}
                    local sw  = CreateReceiveMessageForLeafId(idC("TONE" .. i .. "_SW"))
                    local sel = CreateReceiveMessageForLeafId(idC("TONE" .. i .. "_SEL"))
                    if sw  then table.insert(msgs, sw)  end
                    if sel then table.insert(msgs, sel) end
                    return msgs
                end,
            }
        end

        -- Common section (Partial Switch/Select are in the per-partial Ctrl sections above)
        local k = "Part " .. pn .. " SN-S Common"
        local params = {
            -- Common
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
            {type="select", id=idC("SYNC_RING_SEL"), name=get("Ring / Sync"),       default=0,   options=SyncRing},
            {type="toggle", id=idC("TFX_SW"),        name=get("TFX Switch"),        default=1,   min=get(0),   max=get(1)},
            {type="toggle", id=idC("UNISON"),        name=get("Unison Switch"),     default=0,   min=get(0),   max=get(1)},
            {type="select", id=idC("UNISON_SIZE"),   name=get("Unison Size"),       default=3,   options=UnisonSize},
            {type="range",  id=idC("ANALOG_FEEL"),   name=get("Analog Feel"),       default=0,   min=get(0),   max=get(127), format="%.0f"},
            {type="range",  id=idC("CROSS_MOD"),     name=get("Wave Shape"),        default=0,   min=get(0),   max=get(127), format="%.0f"},
            {type="range",  id=idC("PHRASE"),        name=get("Phrase Number"),     default=0,   min=get(0),   max=get(65535), format="%.0f"},
            {type="range",  id=idC("PHRASE_OCT"),    name=get("Phrase Oct Shift"),  default=0,   min=get(-3),  max=get(3),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            -- Misc
            {type="separator", id="SNS_MISC_SEP_" .. partNr, name=get("Misc")},
            {type="range",  id=idM("MOD_PRM1"), name=get("Atk Time Interval Sens"),  default=0, min=get(0), max=get(127), format="%.0f"},
            {type="range",  id=idM("MOD_PRM2"), name=get("Rel Time Interval Sens"),  default=0, min=get(0), max=get(127), format="%.0f"},
            {type="range",  id=idM("MOD_PRM3"), name=get("Port Time Interval Sens"), default=0, min=get(0), max=get(127), format="%.0f"},
            {type="select", id=idM("MOD_PRM4"), name=get("Env Loop Mode"),           default=0, options=EnvLoopMode},
            {type="select", id=idM("MOD_PRM5"), name=get("Env Loop Sync Note"),      default=0, options=EnvLoopNote},
            {type="toggle", id=idM("MOD_PRM6"), name=get("Chromatic Portamento"),    default=0, min=get(0), max=get(1)},
        }

        for _, param in ipairs(params) do
            ParameterSetValueWrapper(param)
        end

        main[k] = {
            name = "Part " .. string.format("%02d", partNr) .. " SN-S Common",
            params = params,
            getReceiveValueSysex = function()
                local result = {}
                for _, msg in ipairs(CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPC")) do
                    table.insert(result, msg)
                end
                for _, msg in ipairs(CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPTM")) do
                    table.insert(result, msg)
                end
                return result
            end,
        }
    end
end
