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
    print("(S): " .. Bytes_To_String(sysex))
    return sysex
end

local function onRec(received_msg)
    print("(R): " .. Bytes_To_String(received_msg))
    return true
end

function CreateReceiveMessageForBranch(branch_node_id)
    local leafs = GetLeafNodes(branch_node_id)
    local first_leaf = leafs[1]
    local byteSize = Get_Byte_Size(first_leaf.node.valueByteSizeType)
    local msg = Create_Sysex_Rq1_Message(first_leaf.addr, byteSize)
    local rqmsg = RequestMessage.new()
    rqmsg.sysex = msg
    rqmsg.onMessageReceived = onRec
    return {
        rqmsg
    }
end