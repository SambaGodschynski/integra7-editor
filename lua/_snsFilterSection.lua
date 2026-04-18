require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local FilterMode  = {[0]="BYPASS", [1]="LPF", [2]="BPF", [3]="HPF", [4]="PKG", [5]="LPF2", [6]="LPF3", [7]="LPF4"}
local FilterSlope = {[0]="-12", [1]="-24"}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- x10 scale, centre 64  (FILT_FREQ_KF: -100..+100 step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

function CreateSnsFilterSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 3, 1 do
            local k = "Part " .. string.format("%02d", partNr) .. " SN-S Filter Partial " .. partialNr

            local function p(name)
                return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr .. "-SHPT_" .. name
            end

            local params = {
                {type="select", id=p("FILT_MODE"),      name=get("Filter Mode"),       default=1,   options=FilterMode},
                {type="select", id=p("FILT_SLOPE"),     name=get("Filter Slope"),      default=1,   options=FilterSlope},
                {type="range",  id=p("FILT_FREQ"),      name=get("Cutoff"),            default=127, min=get(0),    max=get(127), format="%.0f"},
                {type="range",  id=p("FILT_FREQ_KF"),   name=get("Cutoff Keyfollow"),  default=0,   min=get(-100), max=get(100), format="%+.0f", toI7Value=i7scale10, toGuiValue=gui10scale},
                {type="range",  id=p("FILT_ENV_VSENS"), name=get("Env Velocity Sens"), default=0,   min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("FILT_RESO"),      name=get("Resonance"),         default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range",  id=p("FILT_ENV_ATK"),   name=get("Env Attack"),        default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range",  id=p("FILT_ENV_DCY"),   name=get("Env Decay"),         default=36,  min=get(0),    max=get(127), format="%.0f"},
                {type="range",  id=p("FILT_ENV_SUS"),   name=get("Env Sustain"),       default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range",  id=p("FILT_ENV_REL"),   name=get("Env Release"),       default=0,   min=get(0),    max=get(127), format="%.0f"},
                {type="range",  id=p("FILT_ENV_DEPTH"), name=get("Env Depth"),         default=0,   min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = "Part " .. string.format("%02d", partNr) .. " SN-S Filter Partial " .. partialNr,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr)
                end,
            }
        end
    end
end
