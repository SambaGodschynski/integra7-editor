package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_integra7"
require "_model"
require "_com"
require "_sna"

local default_device_id = 16 -- TODO: make it configurable


Main = {
    parts = {
        name = "Parts View",
        isOpen = true,
        params = {
            {id="PRM-_PRF-_FP1-NEFP_LEVEL", name="Part 1 Level", min=0, max=127},
        }
    },
}

function CreateSysexMessage(node_id, value)
    local sysex = Create_Sysex_Message_For_NodeId(node_id, value, default_device_id)
    print(Bytes_To_String(sysex))
    return sysex
end

CreateSnaSections(Main)