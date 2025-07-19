require "_integra7"

local default_device_id = 16 -- TODO: make it configurable

I7Data = {}
EmptySysex={0xf0, 0xf7}

function ParameterSetValueWrapper(param)
    param.setValue = function (value)
        I7Data[param.id] = value
        return CreateSysexMessage(param.id, value)
    end
    return param
end

function CreateSysexMessage(node_id, value)
    local sysex = Create_Sysex_Message_For_NodeId(node_id, value, default_device_id)
    print(Bytes_To_String(sysex))
    return sysex
end