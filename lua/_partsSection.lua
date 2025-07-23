require "_com"
require "_sysex"

local p = ParameterSetValueWrapper;
local get = GetWrapper

function CreatePartsSections(main)
    local parts = {
        name = "Parts View",
        isOpen = true,
        sub = {}
    }
    for i = 1, 16, 1 do
        local partName = "Part " .. i
        local subSection = {
            name = partName,
            params = {}
        }
        table.insert(subSection.params,
            p({type="range", id="PRM-_PRF-_FP".. i .."-NEFP_LEVEL", name=get(partName ..  " Level"), min=get(0), max=get(127), default=100})
        )
        table.insert(subSection.params,
            p({type="toggle", id="PRM-_PRF-_FP"..i.."-NEFP_MUTE_SW", name=get(partName ..  " Mute"), min=get(0), max=get(1)})
        )
        table.insert(parts.sub, subSection)
    end
    main["parts"] = parts
end