require "_patchesData"
require "_com"
require "_sysex"

local get = GetWrapper
    -- parts = {
    --     name = "Parts View",
    --     isOpen = true,
    --     params = {
    --         p({type="range", id="PRM-_PRF-_FP1-NEFP_LEVEL", name=get("Part 1 Level"), min=get(0), max=get(127)}),
    --     }
    -- },

local function get_patch_key(patch)
    return patch.type .. " " .. patch.category .. " " .. patch.name
end

local patches = MapArray(I7Patches, function (patch)
    local key = get_patch_key(patch)
    return key
end)

local setValue = function (newKey, partNr)
    local patch = I7Patches[newKey]
    
    local lsb = patch.lsb
    local pc = patch.pc
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
    for partNr = 1, 16, 1 do
        local k = "Patches Part" .. string.format("%02d", partNr)
        local onNewValue = function (v)
            return setValue(v, partNr)
        end
        local patchSel = {
            name = k,
            isOpen = partNr == 1,
            params = {
                {type="select", id="PRM-_PRF-_FP"..partNr.."-NEFP_PAT_BS_MSB", name=get("Patch"), default=0, options = patches, setValue = onNewValue}
            },
        }
        main[k] = patchSel
    end
end
