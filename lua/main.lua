package.path = package.path .. ';/home/samba/workspace/integra7-editor/lua/?.lua' -- TODO

require "_integra7"
require "_model"
require "_com"

local default_device_id = 16 -- TODO: make it configurable


local snaTemplate = {
    name = "SN-A",
    sub = {
        name = "Common",
        params = {
            {id="XXXX1", name="Tone Category"},
            {id="XXXX2", name="Phrase Number"}
        }
    }
}


Main = {
    parts = {
        name = "Parts View",
        isOpen = true,
        params = {
            {id="PRM-_PRF-_FP1-NEFP_LEVEL", name="Part 1 Level"},
        }
    },
}

function CreateSysexMessage(node_id, value)
    local sysex = Create_Sysex_Message_For_NodeId(node_id, value, default_device_id)
    print(Bytes_To_String(sysex))
    return sysex
end

for i = 1, 16, 1 do
    local k = "SNA_" .. string.format("%02d", i)
    local name = k
    local inf = DeepCopy(snaTemplate);
    inf.name = name
    if i==1 then
        inf.isOpen = true
    end
    Main[k] = inf
end