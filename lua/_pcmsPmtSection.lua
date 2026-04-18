require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local StructureType = {[0]="1",[1]="2",[2]="3",[3]="4",[4]="5",[5]="6",[6]="7",[7]="8",[8]="9",[9]="10"}
local Booster       = {[0]="0dB",[1]="+6dB",[2]="+12dB",[3]="+18dB"}
local VeloCtrl      = {[0]="OFF",[1]="ON",[2]="RANDOM",[3]="CYCLE"}

local function idPx(partNr, name)
    return "PRM-_FPART" .. partNr .. "-_PAT-_PX-RFPX_" .. name
end

local function idPc(partNr, name)
    return "PRM-_FPART" .. partNr .. "-_PAT-_PC-RFPC_" .. name
end

function CreatePcmsPmtSections(main)
    for partNr = 1, 16, 1 do

        -- Common PMT params (Structure, Booster, Velocity Control)
        local kCommon = "Part " .. string.format("%02d", partNr) .. " PCM-S PMT Common"
        local commonParams = {
            {type="select", id=idPx(partNr,"STRUCT1"),       name=get("Structure Type 1&2"),  default=0, options=StructureType},
            {type="select", id=idPx(partNr,"STRUCT3"),       name=get("Structure Type 3&4"),  default=0, options=StructureType},
            {type="select", id=idPx(partNr,"BOOST1"),        name=get("Booster 1&2"),         default=0, options=Booster},
            {type="select", id=idPx(partNr,"BOOST3"),        name=get("Booster 3&4"),         default=0, options=Booster},
            {type="select", id=idPx(partNr,"TMT_VELO_CTRL"), name=get("PMT Velocity Control"),default=1, options=VeloCtrl},
            {type="toggle", id=idPc(partNr,"TMT_CTRL_SW"),   name=get("PMT Control Switch"),  default=0, min=get(0), max=get(1)},
        }
        for _, param in ipairs(commonParams) do
            param = ParameterSetValueWrapper(param)
        end
        main[kCommon] = {
            name   = "Part " .. string.format("%02d", partNr) .. " PCM-S PMT Common",
            params = commonParams,
            getReceiveValueSysex = function()
                local msgs = CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PX")
                return Concat(msgs, CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PC"))
            end,
        }

        -- Per-partial PMT params (Key/Velocity ranges)
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S PMT Partial " .. partialNr
            local prefix = "TMT" .. partialNr .. "_"

            local partialParams = {
                {type="toggle", id=idPx(partNr, prefix.."SW"),       name=get("Partial Switch"),      default=0,   min=get(0),   max=get(1)},
                {type="range",  id=idPx(partNr, prefix.."KRANGE_LO"), name=get("Key Range Lower"),    default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."KRANGE_UP"), name=get("Key Range Upper"),    default=127, min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."KFADE_LO"),  name=get("Key Fade Lower"),     default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."KFADE_UP"),  name=get("Key Fade Upper"),     default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."VRANGE_LO"), name=get("Velo Range Lower"),   default=1,   min=get(1),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."VRANGE_UP"), name=get("Velo Range Upper"),   default=127, min=get(1),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."VFADE_LO"),  name=get("Velo Fade Lower"),    default=0,   min=get(0),   max=get(127), format="%.0f"},
                {type="range",  id=idPx(partNr, prefix.."VFADE_UP"),  name=get("Velo Fade Upper"),    default=0,   min=get(0),   max=get(127), format="%.0f"},
            }
            for _, param in ipairs(partialParams) do
                param = ParameterSetValueWrapper(param)
            end
            main[kPartial] = {
                name   = "Part " .. string.format("%02d", partNr) .. " PCM-S PMT Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PX")
                end,
            }
        end
    end
end
