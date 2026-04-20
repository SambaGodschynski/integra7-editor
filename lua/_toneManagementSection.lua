require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

-- Special system addresses (source: js-origin/js/view/scene_base_midi_controller.js)
local REQ_TONE_NAME   = 0x0F000402
local WRITE_ADDR      = 0x0F001000
local WRITE_DONE_ADDR = 0x0F001002
local INIT_DONE_ADDR  = 0x0F001802
local BATCH_SIZE      = 64

-- Tone type definitions (user banks)
-- Source: js-origin/js/view/ui_midi_def.js (userToneMsbBank, userToneInfo)
local typeNames    = {"SN-A", "SN-S", "SN-D", "PCM-S", "PCM-D"}
local typeMsb      = {89, 95, 88, 87, 86}
local typeMaxSlots = {256, 512, 64, 256, 32}

-- Name param IDs per type (indexed 1..5), {id_template, bytesize}
-- Placeholder "P" in template is replaced by part number at runtime
local typeNameParam = {
    {"PRM-_FPARTPP-_SNTONE-_SNTC-SNTC_NAME", 16},  -- SN-A
    {"PRM-_FPARTPP-_SNTONE-_SNTC-SNTC_NAME", 16},  -- SN-S
    {"PRM-_FPARTPP-_KIT-_KC-SDKC_NAME",       16},  -- SN-D
    {"PRM-_FPARTPP-_PAT-_PC-RFPC_NAME",        12},  -- PCM-S
    {"PRM-_FPARTPP-_RHY-_RC-RFRC_NAME",        12},  -- PCM-D
}

local function toneNameParamId(typeIdx, partNr)
    local tmpl = typeNameParam[typeIdx][1]
    return string.gsub(tmpl, "PP", tostring(partNr))
end

local function toneNameByteSize(typeIdx)
    return typeNameParam[typeIdx][2]
end

-- Tone names cache: [typeIdx][slot(0-based)] = display string
local toneNames = {}
for i = 1, 5 do
    toneNames[i] = {}
    for j = 0, typeMaxSlots[i] - 1 do
        toneNames[i][j] = string.format("%d", j + 1)
    end
end

-- Per-part state
local partTypeIdx     = {}  -- 1..5
local partToneSlot    = {}  -- 0-based slot index
local pendingNames    = {}  -- edited tone name per part (nil = no edit)
for i = 1, 16 do
    partTypeIdx[i]  = 1
    partToneSlot[i] = 0
    pendingNames[i] = nil
end

local function parseAddr(bytes)
    return (bytes[8] << 24) | (bytes[9] << 16) | (bytes[10] << 8) | bytes[11]
end


-- Build all batch RequestMessages for reading tone names of all types.
-- Source: js-origin readUserToneName() size encoding:
--   d = bankMsb<<24 | floor(index/2)<<16 | (index%2)*BATCH<<8 | tx_size
local function buildAllNameReceives()
    local msgs = {}
    for ti = 1, 5 do
        local msb      = typeMsb[ti]
        local maxSlots = typeMaxSlots[ti]
        local numFull  = math.floor(maxSlots / BATCH_SIZE)
        local lastSize = maxSlots % BATCH_SIZE
        local total    = numFull + (lastSize > 0 and 1 or 0)

        for bi = 0, total - 1 do
            local count  = (bi < numFull) and BATCH_SIZE or lastSize
            local lsb    = math.floor(bi / 2)
            local pcOff  = (bi % 2) * BATCH_SIZE
            local size   = (msb << 24) | (lsb << 16) | (pcOff << 8) | count

            local msg = RequestMessage.new()
            msg.sysex = Create_Sysex_Rq1_Message(REQ_TONE_NAME, size)
            msg.multiResponse = true
            msg.onMessageReceived = function(bytes)
                if #bytes == 0 then return {EmptyValueChangedMessage} end
                if parseAddr(bytes) ~= REQ_TONE_NAME then return {EmptyValueChangedMessage} end
                -- EOD: first payload bytes all zero
                local isEod = true
                for i = 12, math.min(32, #bytes) do
                    if bytes[i] ~= 0 then isEod = false; break end
                end
                if isEod then return {EmptyValueChangedMessage} end
                -- Parse response: bytes[12]=MSB, [13]=LSB, [14]=PC, [17..32]=name
                local rMsb  = bytes[12]
                local rLsb  = bytes[13]
                local rPc   = bytes[14]
                local slot  = rLsb * 128 + rPc
                local tIdx  = nil
                for i, v in ipairs(typeMsb) do
                    if v == rMsb then tIdx = i; break end
                end
                if tIdx == nil then return {EmptyValueChangedMessage} end
                local name = ""
                for i = 17, 32 do
                    local b = bytes[i]
                    if b ~= nil and b >= 32 and b <= 127 then
                        name = name .. string.char(b)
                    end
                end
                name = string.gsub(name, "%s+$", "")
                if name == "" then name = string.format("%d", slot + 1) end
                toneNames[tIdx][slot] = name
                return {EmptyValueChangedMessage}
            end
            table.insert(msgs, msg)
        end
    end
    return msgs
end

-- Tone option display: "N: Name" or just "N" before receive
local function toneOptionsFn(partNr)
    return function()
        local ti   = partTypeIdx[partNr]
        local max  = typeMaxSlots[ti]
        local opts = {}
        for i = 0, max - 1 do
            local nm = toneNames[ti][i]
            if nm and nm ~= string.format("%d", i + 1) then
                opts[i] = string.format("%d: %s", i + 1, nm)
            else
                opts[i] = string.format("%d", i + 1)
            end
        end
        return opts
    end
end

-- Send Bank Select MSB + LSB + PC to device for a given part/type/slot
local function sendToneLoad(partNr, typeIdx, slot)
    local msb   = typeMsb[typeIdx]
    local lsb   = math.floor(slot / 128)
    local pc    = slot % 128
    local msbId = "PRM-_PRF-_FP" .. partNr .. "-NEFP_PAT_BS_MSB"
    local lsbId = "PRM-_PRF-_FP" .. partNr .. "-NEFP_PAT_BS_LSB"
    local pcId  = "PRM-_PRF-_FP" .. partNr .. "-NEFP_PAT_PC"
    local msgs  = Concat(CreateSysexMessage(msbId, msb), CreateSysexMessage(lsbId, lsb))
    return Concat(msgs, CreateSysexMessage(pcId, pc))
end

local function buildPartInitAction(partNr)
    return function()
        local msg = RequestMessage.new()
        msg.sysex = Create_Sysex_Rq1_Message(0x0F001810, 0x7F7F0000 + ((partNr - 1) << 8))
        msg.onMessageReceived = function(_) return {EmptyValueChangedMessage} end
        return {msg}
    end
end

local function buildToneInitAction(partNr)
    return function()
        local msg = RequestMessage.new()
        msg.sysex = Create_Sysex_Rq1_Message(0x0F001820, 0x7F7F0000 + ((partNr - 1) << 8))
        msg.onMessageReceived = function(_) return {EmptyValueChangedMessage} end
        return {msg}
    end
end

local function buildSoundCtrlInitAction()
    return function()
        local msg = RequestMessage.new()
        msg.sysex = Create_Sysex_Rq1_Message(0x0F001808, 0x7F7F0000)
        msg.onMessageReceived = function(_) return {EmptyValueChangedMessage} end
        return {msg}
    end
end

-- Write current tone to selected user slot.
-- If a name edit is pending, sends name DT1 first, then write RQ1.
-- Size encoding: msb<<24 | lsb<<16 | pc<<8 | part(0-based)
-- Source: js-origin writeTone()
local function buildToneWriteAction(partNr)
    return function()
        local typeIdx  = partTypeIdx[partNr]
        local toneSlot = partToneSlot[partNr]
        local msb   = typeMsb[typeIdx]
        local lsb   = math.floor(toneSlot / 128)
        local pc    = toneSlot % 128
        local part  = partNr - 1  -- 0-based
        local size  = (msb << 24) | (lsb << 16) | (pc << 8) | part

        -- Send pending name DT1 before writing
        local nameToWrite = pendingNames[partNr]
        if nameToWrite ~= nil then
            local nameId    = toneNameParamId(typeIdx, partNr)
            local nameAddr  = Get_Adress(nameId)
            local nameBytes = Value_To_Bytes(nameToWrite, toneNameByteSize(typeIdx))
            local nameSysex = Create_Sysex_Message_For_Payload(nameAddr, nameBytes, GetDeviceId())
            SendDirectMessage(nameSysex)
            toneNames[typeIdx][toneSlot] = string.gsub(nameToWrite, "%s+$", "")
            pendingNames[partNr] = nil
        end

        local writeMsg = RequestMessage.new()
        writeMsg.sysex = Create_Sysex_Rq1_Message(WRITE_ADDR, size)
        writeMsg.onMessageReceived = function(_) return {EmptyValueChangedMessage} end
        return {writeMsg}
    end
end

local function makePartSection(partNr)
    local typeId = string.format("TONE_MGMT_TYPE_%02d", partNr)
    local toneId = string.format("TONE_MGMT_TONE_%02d", partNr)
    local nameId = string.format("TONE_MGMT_NAME_%02d", partNr)

    local params = {
        {type = "separator", id = string.format("TONE_MGMT_SEP_R_%02d", partNr), name = get("READ")},
        -- Type + Tone selectors inline (size > 0 enables inline rendering)
        {
            type    = "select",
            id      = typeId,
            name    = get("Type"),
            size    = 80,
            default = 0,
            min     = function() return 0 end,
            max     = function() return 4 end,
            options = function()
                local opts = {}
                for i, n in ipairs(typeNames) do opts[i - 1] = n end
                return opts
            end,
            setValue = function(value)
                local idx = math.tointeger(value) + 1
                partTypeIdx[partNr]  = idx
                partToneSlot[partNr] = 0
                pendingNames[partNr] = nil
                I7Data[typeId] = value
                I7Data[toneId] = 0
                return sendToneLoad(partNr, idx, 0)
            end,
        },
        {
            type    = "select",
            id      = toneId,
            name    = get("Tone"),
            size    = 200,
            default = 0,
            min     = function() return 0 end,
            max     = function() return typeMaxSlots[partTypeIdx[partNr]] - 1 end,
            options = toneOptionsFn(partNr),
            setValue = function(value)
                local slot = math.tointeger(value)
                partToneSlot[partNr] = slot
                pendingNames[partNr] = nil
                I7Data[toneId] = value
                return sendToneLoad(partNr, partTypeIdx[partNr], slot)
            end,
        },
        {type = "separator", id = string.format("TONE_MGMT_SEP_%02d", partNr), name = get("WRITE")},
        -- Tone name: editable before writing
        {
            type              = "inputtext",
            id                = nameId,
            name              = get("Name"),
            stringValue       = "",
            stringValueGetter = function()
                if pendingNames[partNr] ~= nil then return pendingNames[partNr] end
                local nm = toneNames[partTypeIdx[partNr]][partToneSlot[partNr]]
                return nm or ""
            end,
            setStringValue = function(str)
                pendingNames[partNr] = str
                return {}
            end,
        },
        -- Part Init
        {
            type      = "action",
            id        = string.format("TONE_MGMT_PART_INIT_%02d", partNr),
            name      = get("Part Init"),
            getAction = buildPartInitAction(partNr),
        },
        -- Tone Init
        {
            type      = "action",
            id        = string.format("TONE_MGMT_TONE_INIT_%02d", partNr),
            name      = get("Tone Init"),
            getAction = buildToneInitAction(partNr),
        },
        -- Tone Write: sends name DT1 (if edited) then writes tone to selected user slot
        {
            type      = "action",
            id        = string.format("TONE_MGMT_WRITE_%02d", partNr),
            name      = get("Write"),
            getAction = buildToneWriteAction(partNr),
        },
    }

    return {
        name   = string.format("Part %d", partNr),
        params = params,
    }
end

function CreateToneManagementSection(main)
    local topParams = {
        -- Sound Ctrl Init: global reset of MIDI controllers (pitch bend, mod, expression, ...)
        {
            type      = "action",
            id        = "TONE_MGMT_SOUND_CTRL_INIT",
            name      = get("Sound Ctrl Init"),
            getAction = buildSoundCtrlInitAction(),
        },
    }

    local grp = {}
    for i = 1, 16 do
        table.insert(grp, makePartSection(i))
    end

    main["Tone Management"] = {
        name     = "Tone Management",
        accordion = true,
        params    = topParams,
        grp       = grp,
        getReceiveValueSysex = function()
            return buildAllNameReceives()
        end,
    }
end
