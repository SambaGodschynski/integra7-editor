require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

-- ROMID values (source: expansion_name.js / Roland Integra-7 MIDI implementation)
local ExpansionOptions = {
    [0]  = "OFF",
    [1]  = "SRX-01 : Dynamic Drum Kits",
    [2]  = "SRX-02 : Concert Piano",
    [3]  = "SRX-03 : Studio SRX",
    [4]  = "SRX-04 : Symphonique Strings",
    [5]  = "SRX-05 : Supreme Dance",
    [6]  = "SRX-06 : Complete Orchestra",
    [7]  = "SRX-07 : Ultimate Keys",
    [8]  = "SRX-08 : Platinum Trax",
    [9]  = "SRX-09 : World Collection",
    [10] = "SRX-10 : Big Brass Ensemble",
    [11] = "SRX-11 : Complete Piano",
    [12] = "SRX-12 : Classic EPs",
    [13] = "ExSN1 : Ethnic",
    [14] = "ExSN2 : Wood Winds",
    [15] = "ExSN3 : Session",
    [16] = "ExSN4 : A.Guitar",
    [17] = "ExSN5 : Brass",
    [18] = "ExSN6 : SFX",
    [19] = "ExPCM : HQ GM2 + PCM Sound Collection",
}

local SlotNames = { "Slot A", "Slot B", "Slot C", "Slot D" }

-- Model IDs for the Studio Set Common NEFC_EXP_SLOT params
local SlotParamIds = {
    "PRM-_PRF-_FC-NEFC_EXP_SLOT1_ROMID",
    "PRM-_PRF-_FC-NEFC_EXP_SLOT2_ROMID",
    "PRM-_PRF-_FC-NEFC_EXP_SLOT3_ROMID",
    "PRM-_PRF-_FC-NEFC_EXP_SLOT4_ROMID",
}

-- Special Integra-7 system addresses (outside the normal model tree)
-- JS source: scene_base_midi_controller.js
local READ_EXP_ADDR = 0x0F000010  -- RQ1 size=0 -> device returns 4 ROMID bytes
local LOAD_EXP_ADDR = 0x0F003000  -- RQ1 with [a,b,c,d] packed as size -> triggers load
local LOAD_EXP_DONE = 0x0F003002  -- DT1 response address when load is complete

-- Lua-side mirror of the currently pending slot values (before Load)
local currentSlots = {0, 0, 0, 0}

-- Last state received from the device; nil = not yet fetched
local lastReadSlots = nil

local function slotsChangedFromDevice()
    if lastReadSlots == nil then return false end
    for i = 1, 4 do
        if currentSlots[i] ~= lastReadSlots[i] then return true end
    end
    return false
end

-- Parse the 4-byte address from a raw SysEx DT1 response (Lua 1-based table):
-- F0 41 dev 00 00 64 12 aa bb cc dd data... checksum F7
local function parseAddr(bytes)
    return (bytes[8] << 24) | (bytes[9] << 16) | (bytes[10] << 8) | bytes[11]
end

-- Build the load SysEx: RQ1 to LOAD_EXP_ADDR, 4 ROMIDs packed as the 4-byte size field.
local function buildLoadSysex()
    local packed = currentSlots[1] * (1 << 24)
                 + currentSlots[2] * (1 << 16)
                 + currentSlots[3] * (1 <<  8)
                 + currentSlots[4]
    return Create_Sysex_Rq1_Message(LOAD_EXP_ADDR, packed)
end

-- Build a single RequestMessage to read all 4 slots via 0x0F000010.
-- JS uses size=0 for this special address; device always responds with 4 bytes.
-- Updates currentSlots, lastReadSlots, and the UI (used by getReceiveValueSysex).
local function buildReadMessage()
    local rqmsg = RequestMessage.new()
    rqmsg.sysex = Create_Sysex_Rq1_Message(READ_EXP_ADDR, 0)
    rqmsg.onMessageReceived = function(bytes)
        if parseAddr(bytes) ~= READ_EXP_ADDR then
            return {EmptyValueChangedMessage}
        end
        local result = {}
        lastReadSlots = {0, 0, 0, 0}
        for i = 1, 4, 1 do
            local romId = bytes[11 + i]
            currentSlots[i] = romId
            lastReadSlots[i] = romId
            local msg = ValueChangedMessage.new()
            msg.id      = SlotParamIds[i]
            msg.i7Value = romId
            table.insert(result, msg)
        end
        return result
    end
    return rqmsg
end

-- Silent read: updates only lastReadSlots without changing currentSlots or the UI.
-- Used by getAction to check device state before deciding whether to load.
local function buildSilentReadMessage()
    local rqmsg = RequestMessage.new()
    rqmsg.sysex = Create_Sysex_Rq1_Message(READ_EXP_ADDR, 0)
    rqmsg.onMessageReceived = function(bytes)
        if parseAddr(bytes) ~= READ_EXP_ADDR then
            return {EmptyValueChangedMessage}
        end
        lastReadSlots = {}
        for i = 1, 4 do
            lastReadSlots[i] = bytes[11 + i]
        end
        return {EmptyValueChangedMessage}
    end
    return rqmsg
end

function CreateExpansionSection(main)
    local params = {}

    for i = 1, 4, 1 do
        local slotIndex = i
        local param = {
            type    = "select",
            id      = SlotParamIds[i],
            name    = get(SlotNames[i]),
            default = 0,
            options = ExpansionOptions,
            -- Only update local state; Load button triggers the actual device command.
            setValue = function(value)
                currentSlots[slotIndex] = math.tointeger(value)
                I7Data[SlotParamIds[slotIndex]] = value
                return EmptySysex
            end,
        }
        table.insert(params, param)
    end

    -- Load button: sends the load command, waits for completion, then reads back.
    local loadAction = {
        type   = "action",
        id     = "EXP_LOAD_ACTION",
        name   = get("Load Expansions"),
        getAction = function()
            -- Step 1: silently read device state
            local readMsg = buildSilentReadMessage()
            -- Step 2 (onDone): compare and conditionally load
            readMsg.onDone = function()
                if lastReadSlots ~= nil and not slotsChangedFromDevice() then
                    return {}  -- device already has our state, nothing to do
                end
                -- lastReadSlots == nil means read gave no usable response; fall through to load
                local loadMsg = RequestMessage.new()
                loadMsg.sysex = buildLoadSysex()
                loadMsg.onMessageReceived = function(_bytes)
                    return {EmptyValueChangedMessage}
                end
                -- After load command is acknowledged, read back to confirm device is ready
                -- and update the UI (device may still be loading; this acts as a sync point)
                loadMsg.onDone = function()
                    return {buildReadMessage()}
                end
                return {loadMsg}
            end
            return {readMsg}
        end,
    }
    table.insert(params, loadAction)

    main["Expansion Slots"] = {
        name = "Expansion Slots",
        params = params,
        -- Read the current expansion slot assignments from the Studio Set Common params.
        getReceiveValueSysex = function()
            return {buildReadMessage()}
        end,
    }
end
