package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_integra7"
require "_model"
local default_device_id = 16 -- TODO: make it configurable

Main = {
    parts = {
        name = "Parts View",
        params = {
            {id="PRM-_PRF-_FP1-NEFP_LEVEL", name="Part 1 Level"},
        }
    },
    sna = {
        name = "SN-A",
        params = {
            {id="XXXX1", name="Tone Category"},
            {id="XXXX2", name="Phrase Number"}
        }
    },
    snd = {
        name = "SN-D",
        params = {
            {id="XXXX1", name="Tone Category"},
            {id="XXXX2", name="Phrase Number"}
        }
    }
}

function CreateSysexMessage(node_id, value)
    local sysex = Create_Sysex_Message_For_NodeId(node_id, value, default_device_id)
    print(Bytes_To_String(sysex))
    return sysex
end