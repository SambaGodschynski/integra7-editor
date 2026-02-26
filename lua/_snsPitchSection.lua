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

function CreateSnsPitchSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 3, 1 do
            local k = "Part " .. string.format("%02d", partNr) .. " SN-S Pitch Partial " .. partialNr

            local function p(name)
                return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr .. "-SHPT_" .. name
            end

            local params = {
                {type="range", id=p("OSC_PIT_CRS"),    name=get("Coarse Pitch"),     default=0, min=get(-24), max=get(24),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("OSC_PIT_DETUNE"), name=get("Detune"),           default=0, min=get(-50), max=get(50),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("OSC_PENV_ATK"),   name=get("Pitch Env Attack"), default=0, min=get(0),   max=get(127), format="%.0f"},
                {type="range", id=p("OSC_PENV_DCY"),   name=get("Pitch Env Decay"),  default=0, min=get(0),   max=get(127), format="%.0f"},
                {type="range", id=p("OSC_PENV_DEPTH"), name=get("Pitch Env Depth"),  default=0, min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = "Part " .. partNr .. " SN-S Pitch Partial " .. partialNr,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPT" .. partialNr)
                end,
            }
        end
    end
end
