require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

function CreateSndCommonSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " SN-D Common"

        local function idC(name)
            return "PRM-_FPART" .. partNr .. "-_KIT-_KC-SDKC_" .. name
        end

        local params = {
            {type="range",  id=idC("LEVEL"),         name=get("Kit Level"),      default=100, min=get(0), max=get(127)},
            {type="range",  id=idC("AMBIENCE_LEVEL"), name=get("Ambience Level"), default=64,  min=get(0), max=get(127)},
            {type="range",  id=idC("PHRASE"),         name=get("Phrase Number"),  default=0,   min=get(0), max=get(127)},
            {type="toggle", id=idC("TFX_SW"),         name=get("TFX Switch"),     default=1,   min=get(0), max=get(1)},
        }

        for _, param in ipairs(params) do
            ParameterSetValueWrapper(param)
        end

        main[k] = {
            name = k,
            params = params,
            getReceiveValueSysex = function()
                return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_KIT-_KC")
            end,
        }
    end
end
