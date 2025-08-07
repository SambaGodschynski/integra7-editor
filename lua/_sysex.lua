require "_integra7"

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

function BytesToIntValue(bytes)
    local result = 0
    for i = 1, #bytes do
        result = result | (bytes[i] << ((#bytes - i) * 8))
    end
    return result
end

EmptyValueChangedMessage = ValueChangedMessage.new()

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
            changeMessage.i7Value = BytesToIntValue(response.payload)
            return {changeMessage}
        end
        local rqmsg = RequestMessage.new()
        rqmsg.sysex = msg
        rqmsg.onMessageReceived = onRec
        table.insert(result, rqmsg)
    end
    return result
end