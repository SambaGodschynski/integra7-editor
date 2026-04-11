require "_sysex"
require "_com"

local get = GetWrapper

function CreateMixerSection(main)
    main["Mixer"] = {
        name   = "Mixer",
        layout = "mixer",
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            local suffixes = {"NEFP_LEVEL", "NEFP_PAN", "NEFP_CHO_SEND", "NEFP_REV_SEND", "NEFP_MUTE_SW"}
            for i = 1, 16, 1 do
                local fp = "PRM-_PRF-_FP" .. i .. "-"
                for _, suffix in ipairs(suffixes) do
                    local msg = CreateReceiveMessageForLeafId(fp .. suffix)
                    if msg then table.insert(result, msg) end
                end
            end
            local msg = CreateReceiveMessageForLeafId("PRM-_PRF-_FC-NEFC_SOLO_PART")
            if msg then table.insert(result, msg) end
            return result
        end,
    }
end
