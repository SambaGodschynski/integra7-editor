require "math"
require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

function CreatePcmdCommonSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " PCM-D Common"

        local function idC(name)
            return "PRM-_FPART" .. partNr .. "-_RHY-_RC-RFRC_" .. name
        end

        local function idC2(name)
            return "PRM-_FPART" .. partNr .. "-_RHY-_RC2-RFRC2_" .. name
        end

        local params = {
            {type="range",  id=idC("LEVEL"),   name=get("Rhythm Level"), default=127, min=get(0), max=get(127)},
            {type="toggle", id=idC2("TFX_SW"), name=get("TFX Switch"),   default=1,   min=get(0), max=get(1)},
        }

        for _, param in ipairs(params) do
            ParameterSetValueWrapper(param)
        end

        main[k] = {
            name = k,
            params = params,
            getReceiveValueSysex = function()
                local msgsRC  = CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_RHY-_RC")
                local msgsRC2 = CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_RHY-_RC2")
                return Concat(msgsRC, msgsRC2)
            end,
        }
    end
end
