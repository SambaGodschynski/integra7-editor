require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- x10 scale, centre 64  (BIAS_LEVEL: -100..+100 step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

function CreateSnsAmpSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 3, 1 do
            local k = "Part " .. string.format("%02d", partNr) .. " SN-S Amp Partial " .. partialNr

            local function p(name)
                return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr .. "-SHPT_" .. name
            end

            local params = {
                {type="range", id=p("AMP_LEVEL"),       name=get("AMP Level"),          default=100, min=get(0),    max=get(127), format="%.0f"},
                {type="range", id=p("AMP_LEVEL_VSENS"), name=get("Level Velocity Sens"), default=19,  min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("AMP_ENV_ATK"),     name=get("AMP Env Attack"),      default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range", id=p("AMP_ENV_DCY"),     name=get("AMP Env Decay"),       default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range", id=p("AMP_ENV_SUS"),     name=get("AMP Env Sustain"),     default=127, min=get(0),    max=get(127), format="%.0f"},
                {type="range", id=p("AMP_ENV_REL"),     name=get("AMP Env Release"),     default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range", id=p("AMP_PAN"),         name=get("Pan"),                 default=0,   min=get(-64),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("BIAS_LEVEL"),      name=get("AMP Keyfollow"),       default=0,   min=get(-100), max=get(100), format="%+.0f", toI7Value=i7scale10,    toGuiValue=gui10scale},
                {type="range", id=p("AFT_CUTOFF_SENS"), name=get("Cutoff Aftertouch"),   default=9,   min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("AFT_LEVEL_SENS"),  name=get("Level Aftertouch"),    default=10,  min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = "Part " .. string.format("%02d", partNr) .. " SN-S Amp Partial " .. partialNr,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr)
                end,
            }
        end
    end
end
