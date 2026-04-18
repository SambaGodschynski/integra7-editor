require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local VCurve  = {[0]="FIXED",[1]="1",[2]="2",[3]="3",[4]="4",[5]="5",[6]="6",[7]="7"}
local BiasDir = {[0]="LOWER",[1]="UPPER",[2]="LOWER&UPPER",[3]="ALL"}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- ×10 scale with centre at 64  (BIAS_LEVEL, PAN_KF: -100..+100 step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

function CreatePcmsTvaSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S TVA Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            local partialParams = {
                {type="range",  id=p("LEVEL"),       name=get("Partial Level"),    default=127, min=get(0),    max=get(127), format="%.0f"},
                {type="select", id=p("LEVEL_VCRV"),  name=get("Level V-Curve"),    default=1,   options=VCurve},
                {type="range",  id=p("LEVEL_VSENS"), name=get("Level V-Sens"),     default=0,   min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("BIAS_LEVEL"),  name=get("Bias Level"),       default=0,   min=get(-100), max=get(100), format="%+.0f", toI7Value=i7scale10,    toGuiValue=gui10scale},
                {type="range",  id=p("BIAS_POS"),    name=get("Bias Position"),    default=60,  min=get(0),    max=get(127), format="%.0f"},
                {type="select", id=p("BIAS_DIR"),    name=get("Bias Direction"),   default=3,   options=BiasDir},
                {type="range",  id=p("PAN"),         name=get("Pan"),              default=0,   min=get(-64),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range",  id=p("PAN_KF"),      name=get("Pan Keyfollow"),    default=0,   min=get(-100), max=get(100), format="%+.0f", toI7Value=i7scale10,    toGuiValue=gui10scale},
                {type="range",  id=p("PAN_RND"),     name=get("Random Pan"),       default=0,   min=get(0),    max=get(63),  format="%.0f"},
                {type="range",  id=p("PAN_ALT"),     name=get("Alternate Pan"),    default=0,   min=get(-63),  max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }
            for _, param in ipairs(partialParams) do
                ParameterSetValueWrapper(param)
            end
            main[kPartial] = {
                name   = "Part " .. string.format("%02d", partNr) .. " PCM-S TVA Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
