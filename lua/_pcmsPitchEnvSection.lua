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

function CreatePcmsPitchEnvSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S Pitch Env Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            local partialParams = {
                {type="range", id=p("PENV_DEPTH"),    name=get("Pitch Env Depth"),      default=0,  min=get(-12), max=get(12),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_VSENS"),    name=get("Pitch Env V-Sens"),      default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_T1_VSENS"), name=get("Pitch Env T1 V-Sens"),   default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_T4_VSENS"), name=get("Pitch Env T4 V-Sens"),   default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_L0"),       name=get("Pitch Env Level 0"),      default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_T1"),       name=get("Pitch Env Time 1"),       default=0,  min=get(0),   max=get(127), format="%.0f"},
                {type="range", id=p("PENV_L1"),       name=get("Pitch Env Level 1"),      default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_T2"),       name=get("Pitch Env Time 2"),       default=0,  min=get(0),   max=get(127), format="%.0f"},
                {type="range", id=p("PENV_L2"),       name=get("Pitch Env Level 2"),      default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_T3"),       name=get("Pitch Env Time 3"),       default=0,  min=get(0),   max=get(127), format="%.0f"},
                {type="range", id=p("PENV_L3"),       name=get("Pitch Env Level 3"),      default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_T4"),       name=get("Pitch Env Time 4"),       default=0,  min=get(0),   max=get(127), format="%.0f"},
                {type="range", id=p("PENV_L4"),       name=get("Pitch Env Level 4"),      default=0,  min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PENV_TKF"),      name=get("Pitch Env Time KF"),      default=0,  min=get(-10), max=get(10),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }
            for _, param in ipairs(partialParams) do
                param = ParameterSetValueWrapper(param)
            end
            main[kPartial] = {
                name   = "Part " .. partNr .. " PCM-S Pitch Env Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
