require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end

function CreatePcmsPitchSections(main)
    for partNr = 1, 16, 1 do

        -- Common: Pitch Bend Range (from _PC)
        local kCommon = "Part " .. string.format("%02d", partNr) .. " PCM-S Pitch"
        local commonParams = {
            {type="range", id="PRM-_FPART"..partNr.."-_PAT-_PC-RFPC_BEND_RANGE_UP", name=get("Pitch Bend Range Up"),   default=2, min=get(0), max=get(48), format="%.0f"},
            {type="range", id="PRM-_FPART"..partNr.."-_PAT-_PC-RFPC_BEND_RANGE_DW", name=get("Pitch Bend Range Down"), default=2, min=get(0), max=get(48), format="%.0f"},
        }
        for _, param in ipairs(commonParams) do
            param = ParameterSetValueWrapper(param)
        end
        main[kCommon] = {
            name   = "Part " .. string.format("%02d", partNr) .. " PCM-S Pitch",
            params = commonParams,
            getReceiveValueSysex = function()
                return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PC")
            end,
        }

        -- Per-partial: Coarse, Fine, Random, Keyfollow (from _PTy)
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S Pitch Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            local partialParams = {
                {type="range", id=p("PIT_CRS"),  name=get("Coarse Tune"),         default=0,   min=get(-48), max=get(48),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PIT_FINE"), name=get("Fine Tune"),            default=0,   min=get(-50), max=get(50),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("PIT_RND"),  name=get("Random Pitch Depth"),   default=0,   min=get(0),   max=get(30),  format="%.0f"},
                {type="range", id=p("PIT_KF"),   name=get("Pitch Keyfollow"),      default=10,  min=get(-20), max=get(20),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }
            for _, param in ipairs(partialParams) do
                param = ParameterSetValueWrapper(param)
            end
            main[kPartial] = {
                name   = "Part " .. string.format("%02d", partNr) .. " PCM-S Pitch Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
