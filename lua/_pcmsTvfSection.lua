require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local FilterType = {[0]="OFF",[1]="LPF",[2]="BPF",[3]="HPF",[4]="PKG",[5]="LPF2",[6]="LPF3"}
local VCurve     = {[0]="FIXED",[1]="1",[2]="2",[3]="3",[4]="4",[5]="5",[6]="6",[7]="7"}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- ×10 scale with centre at 64 (CUTOFF_KF: -200..+200, step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

function CreatePcmsTvfSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S TVF Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            local partialParams = {
                {type="select", id=p("FILTER_TYPE"),  name=get("Filter Type"),       default=1, options=FilterType},
                {type="range",  id=p("CUTOFF"),        name=get("Cutoff Freq"),        default=127, min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=p("RESO"),          name=get("Resonance"),          default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="select", id=p("CUTOFF_VCRV"),   name=get("Cutoff V-Curve"),     default=1,   options=VCurve},
                {type="range",  id=p("CUTOFF_VSENS"),  name=get("Cutoff V-Sens"),      default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("CUTOFF_KF"),     name=get("Cutoff KF"),          default=0,   min=get(-200),max=get(200), format="%+.0f", toI7Value=i7scale10,     toGuiValue=gui10scale},
                {type="range",  id=p("RESO_VSENS"),    name=get("Resonance V-Sens"),   default=0,   min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }
            for _, param in ipairs(partialParams) do
                ParameterSetValueWrapper(param)
            end
            main[kPartial] = {
                name   = "Part " .. string.format("%02d", partNr) .. " PCM-S TVF Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
