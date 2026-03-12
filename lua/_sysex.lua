require "_integra7"
require "_model"

local default_device_id = 16 -- TODO: make it configurable

function GetDeviceId()
    return default_device_id
end

I7Data = {}
EmptySysex={0xf0, 0xf7}

function ParameterSetValueWrapper(param, visitor)
    param.setValue = function (value)
        I7Data[param.id] = value
        if visitor ~= nil then
            visitor(value)
        end
        return CreateSysexMessage(param.id, value)
    end
    return param
end

function CreateSysexMessage(node_id, value)
    local sysex = Create_Sysex_Message_For_NodeId(node_id, value, default_device_id)
    print("(S): " .. Bytes_To_String(sysex))
    return sysex
end

local function getResponseData(msg)
    local responseOverhead = 11;
    if #msg - responseOverhead <= 0 then
        return nil
    end
    local response = {}
    local i = 1
    local ipp = function ()
        local tmpi = i
        i = i + 1
        return tmpi
    end
    ipp() -- 0xF0
    ipp() -- 0x41
    response.deviceId = msg[ipp()]
    response.modelId = (msg[ipp()] << 16) + (msg[ipp()] << 8) + msg[ipp()];
    response.requestType = msg[ipp()];
    response.addr = (msg[ipp()] << 24) + (msg[ipp()] << 16) + (msg[ipp()] << 8) + msg[ipp()];
    response.payload = {}
    local playload_length = #msg - 1 - 1 -- -f7 -checksum
    table.move(msg, ipp(), playload_length, 1, response.payload)
    return response
end

EmptyValueChangedMessage = ValueChangedMessage.new()

-- Creates a single RQ1 RequestMessage for one leaf node by its full param ID.
-- Returns nil if the node is not found or is not a leaf.
function CreateReceiveMessageForLeafId(node_id)
    local nodeinfo = Get_Node(node_id)
    if nodeinfo == nil then
        return nil
    end
    if nodeinfo.node.children ~= nil then
        return nil
    end
    local byteSize = Get_Byte_Size(nodeinfo.node.valueByteSizeType)
    local sysex = Create_Sysex_Rq1_Message(nodeinfo.addr, byteSize)
    local rqmsg = RequestMessage.new()
    rqmsg.sysex = sysex
    rqmsg.onMessageReceived = function(received_msg)
        local response = getResponseData(received_msg)
        if response == nil or response.addr ~= nodeinfo.addr then
            return {EmptyValueChangedMessage}
        end
        local vcm = ValueChangedMessage.new()
        vcm.id = node_id
        vcm.i7Value = Bytes_To_Value(response.payload)
        return {vcm}
    end
    return rqmsg
end

local receiveHandlers = {}

function AddReceiveHandler(handler)
    table.insert(receiveHandlers, handler)
end

local function notifyHandlers(leaf, response)
    local result = {}
    for _, handler in ipairs(receiveHandlers) do
        local valueChangedMsg = handler(leaf, response)
        if valueChangedMsg ~= nil then
            table.insert(result, valueChangedMsg)
        end
    end
    return result
end

function CreateReceiveMessageForBranch(branch_node_id)
    local leafs = GetLeafNodes(branch_node_id)
    local result = {}
    for _, leaf in ipairs(leafs) do
        local byteSize = Get_Byte_Size(leaf.node.valueByteSizeType)
        local msg = Create_Sysex_Rq1_Message(leaf.addr, byteSize)
        local changeMessage = ValueChangedMessage.new()
        local function onRec(received_msg)
            local response = getResponseData(received_msg)
            if #response == nil or response.addr ~= leaf.addr then
                return {EmptyValueChangedMessage}
            end
            local handledMessages = notifyHandlers(leaf, response)
            if handledMessages ~=nil and #handledMessages > 0 then
                return handledMessages
            end
            changeMessage.id = leaf.fullid
            changeMessage.i7Value = Bytes_To_Value(response.payload)
            return {changeMessage}
        end
        local rqmsg = RequestMessage.new()
        rqmsg.sysex = msg
        rqmsg.onMessageReceived = onRec
        table.insert(result, rqmsg)
    end
    return result
end