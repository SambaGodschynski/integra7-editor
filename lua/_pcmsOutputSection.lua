require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

function CreatePcmsOutputSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S Output Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            local partialParams = {
                {type="range", id=p("DRY_SEND"),   name=get("Output Level"),      default=127, min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("CHO_SEND_A"), name=get("Chorus Send Level"), default=0,   min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("REV_SEND_A"), name=get("Reverb Send Level"), default=0,   min=get(0), max=get(127), format="%.0f"},
            }
            for _, param in ipairs(partialParams) do
                ParameterSetValueWrapper(param)
            end
            main[kPartial] = {
                name   = "Part " .. partNr .. " PCM-S Output Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
