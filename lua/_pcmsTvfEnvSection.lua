require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local VCurve = {[0]="FIXED",[1]="1",[2]="2",[3]="3",[4]="4",[5]="5",[6]="6",[7]="7"}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- ×10 scale with centre at 64 (FENV_TKF: -100..+100, step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

function CreatePcmsTvfEnvSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S TVF Env Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            local partialParams = {
                -- Interactive envelope widget
                {type="envelope", id=kPartial.."-ENV", name=get("TVF Envelope"),
                 levelIds={p("FENV_L0"),p("FENV_L1"),p("FENV_L2"),p("FENV_L3"),p("FENV_L4")},
                 timeIds ={p("FENV_T1"),p("FENV_T2"),p("FENV_T3"),p("FENV_T4")},
                 sustainSegment=true},

                -- Visible knobs / selects
                {type="range",  id=p("FENV_DEPTH"),    name=get("TVF Env Depth"),      default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="select", id=p("FENV_VCRV"),     name=get("TVF Env V-Curve"),    default=1,   options=VCurve},
                {type="range",  id=p("FENV_VSENS"),    name=get("TVF Env V-Sens"),     default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("FENV_T1_VSENS"), name=get("TVF Env T1 V-Sns"),  default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("FENV_T4_VSENS"), name=get("TVF Env T4 V-Sns"),  default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("FENV_TKF"),      name=get("TVF Env Time KF"),    default=0,   min=get(-100),max=get(100), format="%+.0f", toI7Value=i7scale10,    toGuiValue=gui10scale},

                -- Hidden L/T params — kept for MIDI receive and envelope widget I/O
                {type="range", id=p("FENV_L0"), name=get(HideParam), default=0,   min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_T1"), name=get(HideParam), default=0,   min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_L1"), name=get(HideParam), default=127, min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_T2"), name=get(HideParam), default=10,  min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_L2"), name=get(HideParam), default=127, min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_T3"), name=get(HideParam), default=10,  min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_L3"), name=get(HideParam), default=127, min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_T4"), name=get(HideParam), default=64,  min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("FENV_L4"), name=get(HideParam), default=0,   min=get(0), max=get(127), format="%.0f"},
            }
            for _, param in ipairs(partialParams) do
                if param.type ~= "envelope" then
                    ParameterSetValueWrapper(param)
                end
            end
            main[kPartial] = {
                name   = "Part " .. partNr .. " PCM-S TVF Env Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
