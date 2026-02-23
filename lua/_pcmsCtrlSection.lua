require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local EnvMode = {[0]="NO-SUS",[1]="SUSTAIN"}
local CtrlSw  = {[0]="OFF",[1]="ON",[2]="REVERSE"}

function CreatePcmsCtrlSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end
            local receiveFunc = function()
                return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
            end

            local kCtrl = "Part " .. string.format("%02d", partNr) .. " PCM-S CTRL Partial " .. partialNr
            local params = {
                {type="select", id=p("ENV_MODE"),  name=get("Env Mode"),      default=1, options=EnvMode},
                {type="toggle", id=p("RX_BEND"),   name=get("Rx Bender"),     default=1, min=get(0), max=get(1)},
                {type="toggle", id=p("RX_EXPR"),   name=get("Rx Expression"), default=1, min=get(0), max=get(1)},
                {type="toggle", id=p("RX_HOLD"),   name=get("Rx Hold-1"),     default=1, min=get(0), max=get(1)},
                {type="toggle", id=p("REDAMP_SW"), name=get("Redamper Sw"),   default=0, min=get(0), max=get(1)},
            }
            for ctrlNr = 1, 4 do
                for swNr = 1, 4 do
                    params[#params + 1] = {
                        type    = "select",
                        id      = p("CTRL"..ctrlNr.."_SW"..swNr),
                        name    = get("CTRL"..ctrlNr.." SW-"..swNr),
                        default = 1,
                        options = CtrlSw,
                    }
                end
            end

            for _, param in ipairs(params) do ParameterSetValueWrapper(param) end
            main[kCtrl] = {
                name                 = "Part " .. partNr .. " PCM-S CTRL Partial " .. partialNr,
                params               = params,
                getReceiveValueSysex = receiveFunc,
            }
        end
    end
end
