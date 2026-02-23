require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local LfoForm     = {[0]="SIN",[1]="TRI",[2]="SAW-UP",[3]="SAW-DW",[4]="SQR",[5]="RND",
                     [6]="BEND-UP",[7]="BEND-DW",[8]="TRP",[9]="S&H",[10]="CHAOS",[11]="VSIN",[12]="STEP"}
local LfoOffset   = {[0]="-100",[1]="-50",[2]="0",[3]="+50",[4]="+100"}
local LfoFadeMode = {[0]="ON-IN",[1]="ON-OUT",[2]="OFF-IN",[3]="OFF-OUT"}
local StepType    = {[0]="TYPE1",[1]="TYPE2"}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- ×10 scale, centre 64  (DELAY_KF: -100..+100 step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

-- Builds LFO params for a given prefix ("LFO1" or "LFO2")
local function lfoParams(pfx, p)
    return {
        {type="select", id=p(pfx.."_FORM"),       name=get("Waveform"),       default=1,  options=LfoForm},
        {type="range",  id=p(pfx.."_RATE"),        name=get("Rate"),           default=92, min=get(0),    max=get(149), format="%.0f"},
        {type="range",  id=p(pfx.."_RATE_DETN"),   name=get("Rate Detune"),    default=0,  min=get(0),    max=get(127), format="%.0f"},
        {type="select", id=p(pfx.."_OFST"),        name=get("Offset"),         default=2,  options=LfoOffset},
        {type="range",  id=p(pfx.."_DELAY"),       name=get("Delay Time"),     default=0,  min=get(0),    max=get(127), format="%.0f"},
        {type="range",  id=p(pfx.."_DELAY_KF"),    name=get("Delay KF"),       default=0,  min=get(-100), max=get(100), format="%+.0f", toI7Value=i7scale10, toGuiValue=gui10scale},
        {type="select", id=p(pfx.."_FADE_MODE"),   name=get("Fade Mode"),      default=0,  options=LfoFadeMode},
        {type="range",  id=p(pfx.."_FADE"),        name=get("Fade Time"),      default=0,  min=get(0),    max=get(127), format="%.0f"},
        {type="toggle", id=p(pfx.."_KEY_TRIG"),    name=get("Key Trigger"),    default=0,  min=get(0),    max=get(1)},
        {type="range",  id=p(pfx.."_PIT_DEPTH"),   name=get("Pitch Depth"),    default=0,  min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
        {type="range",  id=p(pfx.."_TVF_DEPTH"),   name=get("TVF Depth"),      default=0,  min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
        {type="range",  id=p(pfx.."_TVA_DEPTH"),   name=get("TVA Depth"),      default=0,  min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
        {type="range",  id=p(pfx.."_PAN_DEPTH"),   name=get("Pan Depth"),      default=0,  min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
    }
end

function CreatePcmsLfoSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end
            local receiveFunc = function()
                return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
            end

            -- LFO 1
            local kLfo1 = "Part " .. string.format("%02d", partNr) .. " PCM-S LFO1 Partial " .. partialNr
            local params1 = lfoParams("LFO1", p)
            for _, param in ipairs(params1) do ParameterSetValueWrapper(param) end
            main[kLfo1] = {
                name   = "Part " .. partNr .. " PCM-S LFO1 Partial " .. partialNr,
                params = params1,
                getReceiveValueSysex = receiveFunc,
            }

            -- LFO 2
            local kLfo2 = "Part " .. string.format("%02d", partNr) .. " PCM-S LFO2 Partial " .. partialNr
            local params2 = lfoParams("LFO2", p)
            for _, param in ipairs(params2) do ParameterSetValueWrapper(param) end
            main[kLfo2] = {
                name   = "Part " .. partNr .. " PCM-S LFO2 Partial " .. partialNr,
                params = params2,
                getReceiveValueSysex = receiveFunc,
            }

            -- Step LFO
            local kStep = "Part " .. string.format("%02d", partNr) .. " PCM-S Step LFO Partial " .. partialNr
            local stepIds = {}
            for i = 1, 16 do stepIds[i] = p("LFO_STEP"..i) end

            local stepsParams = {
                -- Step Type select (visible — also drives widget visualisation)
                {type="select", id=p("LFO_STEP_TYPE"), name=get("Step Type"), default=0, options=StepType},
                -- Widget: reads stepTypeId for shape, owns stepIds for bar values
                {type="steplfo", id=kStep.."-STEPLFO", name=get("Step LFO"),
                 stepTypeId=p("LFO_STEP_TYPE"), stepIds=stepIds},
            }
            -- Hidden step params (MIDI send/receive)
            for i = 1, 16 do
                stepsParams[#stepsParams + 1] = {
                    type="range", id=p("LFO_STEP"..i), name=get(HideParam),
                    default=0, min=get(-36), max=get(36), format="%+.0f",
                    toI7Value=i7offset(64), toGuiValue=guiOffset(64),
                }
            end
            for _, param in ipairs(stepsParams) do
                if param.type ~= "steplfo" then ParameterSetValueWrapper(param) end
            end
            main[kStep] = {
                name   = "Part " .. partNr .. " PCM-S Step LFO Partial " .. partialNr,
                params = stepsParams,
                getReceiveValueSysex = receiveFunc,
            }
        end
    end
end
