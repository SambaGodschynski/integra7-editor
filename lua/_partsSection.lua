require "_com"
require "_sysex"

local p = ParameterSetValueWrapper;
local get = GetWrapper
local toneTypes = {"SN-A", "SN-S", "SN-D", "PCM-S", "PCM-D"}
local toneTypeValues = {
    { msb=89, lsb=64 },
    { msb=95, lsb=64 },
    { msb=88, lsb=64 },
    { msb=87, lsb=64 },
    { msb=86, lsb=64 },
}

local function partTypeChange(part)
    return function (value)
        local values = toneTypeValues[value]
        local msbId = "PRM-_PRF-_FP" .. tostring(part) .. "-NEFP_PAT_BS_MSB"
        local msbMessage = CreateSysexMessage(msbId, values.msb)

        local lsbId = "PRM-_PRF-_FP" .. tostring(part) .. "-NEFP_PAT_BS_LSB"
        local lsbMessage = CreateSysexMessage(lsbId, values.lsb)

        local pcId = "PRM-_PRF-_FP" .. tostring(part) .. "-NEFP_PAT_PC"
        local pcMessage = CreateSysexMessage(pcId, 0)

        local msg = Concat(msbMessage, lsbMessage)
        msg = Concat(msg, pcMessage)
        return msg
    end
end

-- Collect all RequestMessages for a given part by iterating Main at call time.
-- Uses the "Part NN " prefix to find all matching sections with getReceiveValueSysex.
local function buildReceiveAllAction(partNr)
    local prefix = "Part " .. string.format("%02d", partNr) .. " "
    return function()
        local result = {}
        for key, section in pairs(Main) do
            if string.sub(key, 1, #prefix) == prefix and section.getReceiveValueSysex then
                for _, msg in ipairs(section.getReceiveValueSysex()) do
                    table.insert(result, msg)
                end
            end
        end
        return result
    end
end

function CreatePartsSections(main)
    local parts = {
        name = "Parts View",
        params = {
            {type="loadsysex", id="LOAD_SYSEX", name=get("Load SysEx")}
        },
        grp = {}
    }
    for i = 1, 16, 1 do
        local partName = "Part " .. i
        local subSection = {
            name = partName,
            params = {}
        }
        table.insert(subSection.params,
            p({type="range", id="PRM-_PRF-_FP"..i.."-NEFP_LEVEL", name=get(partName ..  " Level"), min=get(0), max=get(127), default=100})
        )
        table.insert(subSection.params,
            p({type="toggle", id="PRM-_PRF-_FP"..i.."-NEFP_MUTE_SW", name=get(partName ..  " Mute"), min=get(0), max=get(1)})
        )
        table.insert(subSection.params,
            {type="select", id="PRM-_PRF-_FP"..i.."-NEFP_TYPE_DUMMY", name=get(partName.." Type"), default=1, options=toneTypes, setValue=partTypeChange(i)}
        )
        table.insert(subSection.params,
            {type="savesysex", id="SAVE_SYSEX_PART_"..i, name=get("Save SysEx Part "..string.format("%02d", i)),
             partPrefix="Part " .. string.format("%02d", i) .. " "}
        )
        table.insert(parts.grp, subSection)
    end
    main["parts"] = parts
end