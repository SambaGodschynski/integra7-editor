require "math"

local snaTemplate = {
    name = "SN-A",
    sub =
    {
        {
            name = "Common",
            params =
            {
                { 
                    id="PRM-_FPARTxxx-_SNTONE-_SNTC-SNTC_PHRASE_OCT",
                    name="Phrase Oct Shift",
                    min=-3,
                    max=3,
                    default=0,
                    format="%+.0f",
                    toI7Value=function (guiValue)
                        return math.tointeger(64 + guiValue)
                    end
                },
            }
        }
    }
}

function CreateSnaSections(main)
    for i = 1, 16, 1 do
        local k = "SNA_" .. string.format("%02d", i)
        local name = k
        local snaData = DeepCopy(snaTemplate);
        snaData.name = name
        if i==1 then
            snaData.isOpen = true
        end
        for key, subSection in ipairs(snaData.sub) do
            for key, param in ipairs(subSection.params) do
                param.id = CreateId(param.id, i)
            end
        end
        main[k] = snaData
    end
end
