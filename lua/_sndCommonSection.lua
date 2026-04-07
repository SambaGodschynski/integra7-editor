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
    return NoteNames[(note % 12) + 1] .. tostring(octave)
end

local NoteOptions = {}
for note = 27, 88, 1 do
    NoteOptions[note - 27] = noteName(note)   -- [0]="D#1" .. [61]="E6"
end

local FlamCtrl = {
    [0]="OFF",   [1]="FLAM1", [2]="FLAM2", [3]="FLAM3",
    [4]="BUZZ1", [5]="BUZZ2", [6]="BUZZ3", [7]="ROLL",
}

local OutputAssign = {
    [0]="PART",      [1]="COMP+EQ 1", [2]="COMP+EQ 2",
    [3]="COMP+EQ 3", [4]="COMP+EQ 4", [5]="COMP+EQ 5", [6]="COMP+EQ 6",
}

local nodeToFakeSuffix = {
    SDKN_INST_NUM      = "_DrumInstNum",
    SDKN_INST_LEVEL    = "_DrumLevel",
    SDKN_INST_PAN      = "_DrumPan",
    SDKN_INST_CHO_SEND = "_DrumCho",
    SDKN_INST_REV_SEND = "_DrumRev",
    SDKN_PIT_FINE      = "_DrumTune",
    SDKN_ATTACK        = "_DrumAttack",
    SDKN_DECAY         = "_DrumDecay",
    SDKN_BRIGHTNESS    = "_DrumBright",
    SDKN_FLAM_CTRL     = "_DrumFlam",
    SDKN_DYN_RANG      = "_DrumDyn",
    SDKN_STEREO_WIDTH  = "_DrumWidth",
    SDKN_OUTPUT_ASSIGN = "_DrumOutput",
}

local notePartMap = {27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27}

local fakeNoteId = "PRM-_FPARTxxx_DrumNote"

function CreateSndCommonSections(main)
    for partNr = 1, 16, 1 do
        local pn = string.format("%02d", partNr)
        local k  = "Part " .. pn .. " SN-D Instrument"

        local function idC(name)
            return "PRM-_FPART" .. partNr .. "-_KIT-_KC-SDKC_" .. name
        end

        -- Receive handler for drum note params
        local function drumInstHandler(leafNode, response)
            if not IsIdForPart(leafNode.fullid, partNr) then return nil end
            local currentNote = notePartMap[partNr]
            if not string.find(leafNode.fullid, "_KN" .. currentNote .. "-", 1, true) then
                return nil
            end
            local suffix = nodeToFakeSuffix[leafNode.node.id]
            if suffix == nil then return nil end
            local msg = ValueChangedMessage.new()
            msg.id = "PRM-_FPART" .. partNr .. suffix
            msg.i7Value = Bytes_To_Value(response.payload)
            return msg
        end
        AddReceiveHandler(drumInstHandler)

        local function makeDrumSetter(sdknName)
            return function(i7value)
                local note = notePartMap[partNr]
                local realId = "PRM-_FPART" .. partNr .. "-_KIT-_KN" .. note .. "-SDKN_" .. sdknName
                return CreateSysexMessage(realId, i7value)
            end
        end

        local noteSelectId = CreateId(fakeNoteId, partNr)

        local params = {
            -- Common
            {type="range",  id=idC("LEVEL"),          name=get("Kit Level"),      default=100, min=get(0), max=get(127)},
            {type="range",  id=idC("AMBIENCE_LEVEL"),  name=get("Ambience Level"), default=64,  min=get(0), max=get(127)},
            {type="range",  id=idC("PHRASE"),          name=get("Phrase Number"),  default=0,   min=get(0), max=get(127)},
            {type="toggle", id=idC("TFX_SW"),          name=get("TFX Switch"),     default=1,   min=get(0), max=get(1)},
            {
                type="select",
                id=noteSelectId,
                name=get("Drum Note"),
                default=0,
                options=NoteOptions,
                setValue=function(i7value)
                    notePartMap[partNr] = 27 + math.tointeger(i7value)
                    return {}
                end
            },
            {type="range",  id="PRM-_FPART"..partNr.."_DrumInstNum", name=get("Inst Number"),  default=1,   min=get(0),   max=get(512),                                                setValue=makeDrumSetter("INST_NUM")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumLevel",   name=get("Level"),         default=100, min=get(0),   max=get(127),                                                setValue=makeDrumSetter("INST_LEVEL")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumPan",     name=get("Pan"),           default=0,   min=get(-64), max=get(63),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makeDrumSetter("INST_PAN")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumCho",     name=get("Chorus Send"),   default=0,   min=get(0),   max=get(127),                                                setValue=makeDrumSetter("INST_CHO_SEND")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumRev",     name=get("Reverb Send"),   default=64,  min=get(0),   max=get(127),                                                setValue=makeDrumSetter("INST_REV_SEND")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumTune",    name=get("Tune"),          default=0,   min=get(-120),max=get(120),  format="%+.0f", toI7Value=i7offset(128), toGuiValue=guiOffset(128), setValue=makeDrumSetter("PIT_FINE")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumAttack",  name=get("Attack"),        default=100, min=get(0),   max=get(100),  format="%.0f%%",                             setValue=makeDrumSetter("ATTACK")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumDecay",   name=get("Decay"),         default=0,   min=get(-63), max=get(0),    format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makeDrumSetter("DECAY")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumBright",  name=get("Brilliance"),    default=0,   min=get(-15), max=get(12),   format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64), setValue=makeDrumSetter("BRIGHTNESS")},
            {type="select", id="PRM-_FPART"..partNr.."_DrumFlam",    name=get("Variation"),     default=0,   options=FlamCtrl,                                                          setValue=makeDrumSetter("FLAM_CTRL")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumDyn",     name=get("Dynamic Range"), default=32,  min=get(0),   max=get(63),                                                 setValue=makeDrumSetter("DYN_RANG")},
            {type="range",  id="PRM-_FPART"..partNr.."_DrumWidth",   name=get("Stereo Width"),  default=127, min=get(0),   max=get(127),                                                setValue=makeDrumSetter("STEREO_WIDTH")},
            {type="select", id="PRM-_FPART"..partNr.."_DrumOutput",  name=get("Output Assign"), default=0,   options=OutputAssign,                                                      setValue=makeDrumSetter("OUTPUT_ASSIGN")},
        }

        for _, param in ipairs(params) do
            ParameterSetValueWrapper(param)
        end

        main[k] = {
            name = "Part " .. partNr .. " SN-D Instrument",
            params = params,
            getReceiveValueSysex = function()
                local result = {}
                for _, msg in ipairs(CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_KIT-_KC")) do
                    table.insert(result, msg)
                end
                local note = notePartMap[partNr]
                for _, msg in ipairs(CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_KIT-_KN" .. note)) do
                    table.insert(result, msg)
                end
                return result
            end,
        }
    end
end
