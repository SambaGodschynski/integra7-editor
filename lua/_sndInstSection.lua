require "math"
require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local function i7offset(offset) return function(gui) return math.tointeger(offset + gui) end end
local function guiOffset(offset) return function(i7)  return math.tointeger(i7 - offset)  end end

local NoteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}

local function noteName(note)
    local octave = math.floor(note / 12) - 1
    local name = NoteNames[(note % 12) + 1]
    return name .. tostring(octave)
end

local FlamCtrl = {
    [0]="OFF", [1]="FLAM1", [2]="FLAM2", [3]="FLAM3",
    [4]="BUZZ1", [5]="BUZZ2", [6]="BUZZ3", [7]="ROLL",
}

local OutputAssign = {
    [0]="PART",      [1]="COMP+EQ 1", [2]="COMP+EQ 2",
    [3]="COMP+EQ 3", [4]="COMP+EQ 4", [5]="COMP+EQ 5", [6]="COMP+EQ 6",
}

function CreateSndInstSections(main)
    for partNr = 1, 16, 1 do
        local pn = string.format("%02d", partNr)
        for note = 27, 88, 1 do
            local nn      = noteName(note)
            local k       = "Part " .. pn .. " SN-D Inst " .. nn
            local nodeKey = "_KN" .. note

            local function idN(name)
                return "PRM-_FPART" .. partNr .. "-_KIT-" .. nodeKey .. "-SDKN_" .. name
            end

            local params = {
                {type="range",  id=idN("INST_NUM"),      name=get("Inst Number"),   default=1,   min=get(0),   max=get(512)},
                {type="range",  id=idN("INST_LEVEL"),    name=get("Level"),         default=100, min=get(0),   max=get(127)},
                {type="range",  id=idN("INST_PAN"),      name=get("Pan"),           default=0,   min=get(-64), max=get(63),  format="%+.0f", toI7Value=i7offset(64),  toGuiValue=guiOffset(64)},
                {type="range",  id=idN("INST_CHO_SEND"), name=get("Chorus Send"),   default=0,   min=get(0),   max=get(127)},
                {type="range",  id=idN("INST_REV_SEND"), name=get("Reverb Send"),   default=64,  min=get(0),   max=get(127)},
                {type="range",  id=idN("PIT_FINE"),      name=get("Tune"),          default=0,   min=get(-120),max=get(120), format="%+.0f", toI7Value=i7offset(128), toGuiValue=guiOffset(128)},
                {type="range",  id=idN("ATTACK"),        name=get("Attack"),        default=100, min=get(0),   max=get(100), format="%.0f%%"},
                {type="range",  id=idN("DECAY"),         name=get("Decay"),         default=0,   min=get(-63), max=get(0),   format="%+.0f", toI7Value=i7offset(64),  toGuiValue=guiOffset(64)},
                {type="range",  id=idN("BRIGHTNESS"),    name=get("Brilliance"),    default=0,   min=get(-15), max=get(12),  format="%+.0f", toI7Value=i7offset(64),  toGuiValue=guiOffset(64)},
                {type="select", id=idN("FLAM_CTRL"),     name=get("Variation"),     default=0,   options=FlamCtrl},
                {type="range",  id=idN("DYN_RANG"),      name=get("Dynamic Range"), default=32,  min=get(0),   max=get(63)},
                {type="range",  id=idN("STEREO_WIDTH"),  name=get("Stereo Width"),  default=127, min=get(0),   max=get(127)},
                {type="select", id=idN("OUTPUT_ASSIGN"), name=get("Output Assign"), default=0,   options=OutputAssign},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = k,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_KIT-" .. nodeKey)
                end,
            }
        end
    end
end
