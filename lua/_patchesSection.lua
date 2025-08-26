require "_patchesData"
require "_com"
require "_sysex"

local get = GetWrapper

local function get_patch_key(patch, index)
    return string.format("%04i", index) .. " " .. patch.type .. " " .. patch.category .. " " .. patch.name
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
                {type="select", id="PRM-_PRF-_FP"..partNr.."-NEFP_PAT_BS_MSB", name=get("Part " .. partNr), default=0, options = patches, setValue = onNewValue}
            }
        }
        table.insert(section.grp, grp)
    end
    main["presets"] = section
end
