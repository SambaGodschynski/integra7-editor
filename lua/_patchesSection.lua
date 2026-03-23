require "_patchesData"
require "_com"
require "_sysex"

local get = GetWrapper

-- LSB starting offset per SRX board (SRX-01 .. SRX-12)
local srxLsbOffsets = {0, 1, 2, 4, 7, 11, 15, 19, 23, 24, 26, 27}

local function getExpansionTag(msb, lsb)
    if msb == 89 then
        if lsb >= 96 and lsb <= 100 then
            return "[ExSN" .. (lsb - 95) .. "]"
        end
        return ""
    end
    if msb == 88 then
        if lsb == 101 then return "[ExSN6]" end
        return ""
    end
    if msb == 95 or msb == 86 or msb == 87 then return "" end
    if msb == 96 or msb == 97 then return "[ExPCM]" end
    if msb == 120 or msb == 121 then return "[GM2]" end
    if msb == 92 or msb == 93 then
        for i = #srxLsbOffsets, 1, -1 do
            if lsb >= srxLsbOffsets[i] then
                return string.format("[SRX-%02d]", i)
            end
        end
    end
    return ""
end

local function get_patch_key(patch, index)
    local tag = getExpansionTag(patch.msb, patch.lsb)
    local tagPart = tag ~= "" and (tag .. " ") or ""
    return string.format("%04i", index) .. " " .. tagPart .. patch.type .. " " .. patch.category .. " " .. patch.name
end

local patches = MapArray(I7Patches, function (patch, index)
    local key = get_patch_key(patch, index)
    return key
end)

local setValue = function (newKey, partNr)
    local patch = I7Patches[newKey]
    
    local lsb = patch.lsb
    local pc = patch.pc - 1
    local msb = patch.msb
    local msbId = "PRM-_PRF-_FP"..partNr.."-NEFP_PAT_BS_MSB"
    local msbMessage = CreateSysexMessage(msbId, msb)
    local lsbId = "PRM-_PRF-_FP"..partNr.."-NEFP_PAT_BS_LSB"
    local lsbMessage = CreateSysexMessage(lsbId, lsb)
    local pcId = "PRM-_PRF-_FP"..partNr.."-NEFP_PAT_PC"
    local pcMessage = CreateSysexMessage(pcId, pc)

    local msg = Concat(msbMessage, lsbMessage)
    msg = Concat(msg, pcMessage)

    return msg
end

function CreatePatchesSections(main)
    local section = {
        name = "Presets",
        grp = {}
    }
    for partNr = 1, 16, 1 do
        local onNewValue = function (v)
            return setValue(v, partNr)
        end
        local grp = {
            name = "Part " .. string.format("%02d", partNr) .. " Presets",
            params = {
                {type="select", id="PRM-_PRF-_FP"..partNr.."-NEFP_PAT_BS_MSB", name=get("Part " .. partNr), default=1, options = patches, setValue = onNewValue}
            }
        }
        table.insert(section.grp, grp)
    end
    main["presets"] = section
end
