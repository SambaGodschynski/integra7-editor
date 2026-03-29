require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

-- Special system addresses (direct SysEx bytes, not nibbled)
-- Source: js-origin/js/view/scene_base_midi_controller.js
local READ_NAMES_ADDR = 0x0F000302
local WRITE_ADDR      = 0x0F001000
local WRITE_DONE_ADDR = 0x0F001002
local INIT_ADDR       = 0x0F001800
local INIT_DONE_ADDR  = 0x0F001802

local STUDIO_BANK_MSB = 0x55
local NUM_SETS = 64

-- Slot names: populated by Receive Names, initially "Set 1".."Set 64"
local studioSetNames = {}
for i = 1, NUM_SETS do
    studioSetNames[i] = string.format("Set %d", i)
end

-- Pending name edits per slot (nil = no pending edit)
local pendingNames = {}
for i = 1, NUM_SETS do
    pendingNames[i] = nil
end

local function parseAddr(bytes)
    return (bytes[8] << 24) | (bytes[9] << 16) | (bytes[10] << 8) | bytes[11]
end

-- Build Receive All Names request (multi-response: 64 sets + 1 EOD)
-- Response payload layout (bytes after the 4-byte address, 1-based Lua indices 12+):
--   byte 2 of payload (index 14): Studio Set number, 0-based
--   bytes 5-20 of payload (indices 17-32): 16-char ASCII name
-- Source: js-origin/js/communicator/communicator.js + scene_base_midi_controller.js
local function buildReadNamesMessage()
    local rqmsg = RequestMessage.new()
    local size = (STUDIO_BANK_MSB << 24) + (0x00 << 16) + (0x00 << 8) + NUM_SETS
    rqmsg.sysex = Create_Sysex_Rq1_Message(READ_NAMES_ADDR, size)
    rqmsg.multiResponse = true
    rqmsg.onMessageReceived = function(bytes)
        if #bytes == 0 then
            return {EmptyValueChangedMessage}
        end
        if parseAddr(bytes) ~= READ_NAMES_ADDR then
            return {EmptyValueChangedMessage}
        end
        -- EOD: first 21 payload bytes all zero
        local isEod = true
        for i = 12, math.min(32, #bytes) do
            if bytes[i] ~= 0 then
                isEod = false
                break
            end
        end
        if isEod then
            return {EmptyValueChangedMessage}
        end
        local setNum = bytes[14]  -- 0-based (0-63)
        if setNum == nil or setNum < 0 or setNum >= NUM_SETS then
            return {EmptyValueChangedMessage}
        end
        local name = ""
        for i = 17, 32 do
            local b = bytes[i]
            if b ~= nil and b >= 32 and b <= 127 then
                name = name .. string.char(b)
            end
        end
        name = string.gsub(name, "%s+$", "")
        studioSetNames[setNum + 1] = name
        return {EmptyValueChangedMessage}
    end
    return rqmsg
end

-- Build Init action (initializes current Studio Set to defaults)
local function buildInitMessage()
    local msg = RequestMessage.new()
    msg.sysex = Create_Sysex_Rq1_Message(INIT_ADDR, 0x7F7F0000)
    msg.onMessageReceived = function(bytes)
        if parseAddr(bytes) ~= INIT_DONE_ADDR then
            return {EmptyValueChangedMessage}
        end
        return {EmptyValueChangedMessage}
    end
    return msg
end

-- Build Write action for slot i (1-based)
-- If pendingNames[i] is set, sends a DT1 to update the current set name first.
local function buildWriteAction(i)
    local slotNum = i - 1  -- 0-based (0-63)
    return function()
        -- Send name DT1 directly if user changed the name for this slot
        local nameToWrite = pendingNames[i]
        if nameToWrite ~= nil then
            local nameAddr = Get_Adress("PRM-_PRF-_FC-NEFC_NAME")
            local nameBytes = Value_To_Bytes(nameToWrite, 16)
            local nameSysex = Create_Sysex_Message_For_Payload(nameAddr, nameBytes, GetDeviceId())
            SendDirectMessage(nameSysex)
            studioSetNames[i] = nameToWrite
            pendingNames[i] = nil
        end

        -- Write RQ1: writes current Studio Set to the target slot
        local size = (STUDIO_BANK_MSB << 24) + (slotNum << 8) + 0x7F
        local writeMsg = RequestMessage.new()
        writeMsg.sysex = Create_Sysex_Rq1_Message(WRITE_ADDR, size)
        writeMsg.onMessageReceived = function(bytes)
            if parseAddr(bytes) ~= WRITE_DONE_ADDR then
                return {EmptyValueChangedMessage}
            end
            return {EmptyValueChangedMessage}
        end
        return {writeMsg}
    end
end

-- One accordion sub-section per Studio Set slot
local function makeSetSection(i)
    local section = {
        name = string.format("Set %d", i),
        params = {
            {
                type              = "inputtext",
                id                = string.format("STUDIO_NAME_%02d", i),
                name              = get("Name"),
                stringValue       = studioSetNames[i],
                stringValueGetter = function()
                    return pendingNames[i] or studioSetNames[i]
                end,
                setStringValue    = function(str)
                    pendingNames[i] = str
                    return {}
                end,
            },
            {
                type      = "action",
                id        = string.format("STUDIO_WRITE_%02d", i),
                name      = get("Write"),
                getAction = buildWriteAction(i),
            },
        },
    }
    return section
end

function CreateStudioManagementSection(main)
    local topParams = {
        -- Studio Set selector: Bank Select (MSB 85, LSB 0) + Program Change
        {
            type    = "select",
            id      = "STUDIO_SET_SELECT",
            name    = get("Studio Set"),
            default = 0,
            min     = function() return 0 end,
            max     = function() return NUM_SETS - 1 end,
            options = function()
                local opts = {}
                for i = 1, NUM_SETS do
                    opts[i - 1] = string.format("%d: %s", i, studioSetNames[i])
                end
                return opts
            end,
            setValue = function(value)
                local setNum = math.tointeger(value)
                local msbMsg = CreateSysexMessage("PRM-_SETUP-_STP-NESTP_PRF_BS_MSB_SD1", 0x55)
                local lsbMsg = CreateSysexMessage("PRM-_SETUP-_STP-NESTP_PRF_BS_LSB_SD1", 0x00)
                local pcMsg  = CreateSysexMessage("PRM-_SETUP-_STP-NESTP_PRF_PC_SD1", setNum)
                I7Data["STUDIO_SET_SELECT"] = value
                return Concat(Concat(msbMsg, lsbMsg), pcMsg)
            end,
        },
        {
            type      = "action",
            id        = "STUDIO_INIT",
            name      = get("Init"),
            getAction = function()
                return {buildInitMessage()}
            end,
        },
    }

    local grp = {}
    for i = 1, NUM_SETS do
        table.insert(grp, makeSetSection(i))
    end

    main["Studio Management"] = {
        name                  = "Studio Management",
        accordion             = true,
        params                = topParams,
        grp                   = grp,
        getReceiveValueSysex  = function()
            return {buildReadNamesMessage()}
        end,
    }
end
