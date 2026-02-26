require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local EnvLoopMode = {[0]="OFF", [1]="FREE-RUN", [2]="TEMPO-SYNC"}
local EnvLoopNote = {
    [0]="1/16T", [1]="1/8T",  [2]="1/4T",  [3]="1/2T",
    [4]="1/16",  [5]="1/8",   [6]="3/16",  [7]="1/4",
    [8]="3/8",   [9]="1/2",   [10]="3/4",  [11]="1",
    [12]="3/2",  [13]="2",    [14]="3",     [15]="4",
    [16]="6",    [17]="8",    [18]="12",    [19]="16",
    [20]="24",   [21]="32",
}

function CreateSnsMiscSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " SN-S Misc"

        local function idM(name)
            return "PRM-_FPART" .. partNr .. "-_SHPAT-_SHPTM-SHPTM_" .. name
        end

        local params = {
            {type="range",  id=idM("MOD_PRM1"), name=get("Atk Time Interval Sens"),  default=0, min=get(0), max=get(127), format="%.0f"},
            {type="range",  id=idM("MOD_PRM2"), name=get("Rel Time Interval Sens"),  default=0, min=get(0), max=get(127), format="%.0f"},
            {type="range",  id=idM("MOD_PRM3"), name=get("Port Time Interval Sens"), default=0, min=get(0), max=get(127), format="%.0f"},
            {type="select", id=idM("MOD_PRM4"), name=get("Env Loop Mode"),           default=0, options=EnvLoopMode},
            {type="select", id=idM("MOD_PRM5"), name=get("Env Loop Sync Note"),      default=0, options=EnvLoopNote},
            {type="toggle", id=idM("MOD_PRM6"), name=get("Chromatic Portamento"),    default=0, min=get(0), max=get(1)},
        }

        for _, param in ipairs(params) do
            ParameterSetValueWrapper(param)
        end

        main[k] = {
            name = "Part " .. string.format("%02d", partNr) .. " SN-S Misc",
            params = params,
            getReceiveValueSysex = function()
                return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPTM")
            end,
        }
    end
end
