require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local LfoShape    = {[0]="SIN", [1]="TRI", [2]="SAW-UP", [3]="SAW-DW", [4]="SQR", [5]="RND"}
local LfoSyncNote = {
    [0]="1/16T", [1]="1/8T",  [2]="1/4T",  [3]="1/2T",
    [4]="1/16",  [5]="1/8",   [6]="3/16",  [7]="1/4",
    [8]="3/8",   [9]="1/2",   [10]="3/4",  [11]="1",
    [12]="3/2",  [13]="2",    [14]="3",     [15]="4",
    [16]="6",    [17]="8",    [18]="12",    [19]="16",
}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end

function CreateSnsLfoSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 3, 1 do
            local k = "Part " .. string.format("%02d", partNr) .. " SN-S LFO Partial " .. partialNr

            local function p(name)
                return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr .. "-SHPT_" .. name
            end

            local receiveFunc = function()
                return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr)
            end

            local params = {
                -- LFO
                {type="select", id=p("LFO_SHAPE"),      name=get("LFO Shape"),         default=0,  options=LfoShape},
                {type="range",  id=p("LFO_RATE"),        name=get("LFO Rate"),          default=81, min=get(0),   max=get(127), format="%.0f"},
                {type="toggle", id=p("LFO_SYNC_SW"),     name=get("LFO Tempo Sync"),    default=0,  min=get(0),   max=get(1)},
                {type="select", id=p("LFO_SYNC_NOTE"),   name=get("LFO Sync Note"),     default=17, options=LfoSyncNote},
                {type="range",  id=p("LFO_FADE"),        name=get("LFO Fade Time"),     default=0,  min=get(0),   max=get(127), format="%.0f"},
                {type="toggle", id=p("LFO_KEY_TRIG"),    name=get("LFO Key Trigger"),   default=0,  min=get(0),   max=get(1)},
                {type="range",  id=p("LFO_PITCH_MOD"),   name=get("LFO Pitch Depth"),   default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("LFO_FILTER_MOD"),  name=get("LFO Filter Depth"),  default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("LFO_AMP_MOD"),     name=get("LFO Amp Depth"),     default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("LFO_PAN_MOD"),     name=get("LFO Pan Depth"),     default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                -- Mod LFO
                {type="select", id=p("MLFO_SHAPE"),      name=get("ModLFO Shape"),      default=0,  options=LfoShape},
                {type="range",  id=p("MLFO_RATE"),       name=get("ModLFO Rate"),       default=88, min=get(0),   max=get(127), format="%.0f"},
                {type="toggle", id=p("MLFO_SYNC_SW"),    name=get("ModLFO Tempo Sync"), default=0,  min=get(0),   max=get(1)},
                {type="select", id=p("MLFO_SYNC_NOTE"),  name=get("ModLFO Sync Note"),  default=17, options=LfoSyncNote},
                {type="range",  id=p("MLFO_PITCH_MOD"),  name=get("ModLFO Pitch"),      default=16, min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("MLFO_FILTER_MOD"), name=get("ModLFO Filter"),     default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("MLFO_AMP_MOD"),    name=get("ModLFO Amp"),        default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("MLFO_PAN_MOD"),    name=get("ModLFO Pan"),        default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("MLFO_RATE_MOD"),   name=get("ModLFO Rate Ctrl"),  default=18, min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = "Part " .. partNr .. " SN-S LFO Partial " .. partialNr,
                params = params,
                getReceiveValueSysex = receiveFunc,
            }
        end
    end
end
