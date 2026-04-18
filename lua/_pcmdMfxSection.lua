require "math"
require "_mfx"
require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local function idTmpl(mfxId)
    return "PRM-_FPARTxxx-_RHY-_RF-" .. mfxId
end

local mfxNumberPartMap = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

local mfxTypes = MapDict(Mfx_Table, function(type)
    return type.desc
end)

local function get_set_mfx_default_values_sysex(mfxType, part_id, mfx_id)
    local prm = Mfx_Table[mfx_id + 1]
    local payload = {}
    local from = -1
    local nodes = Mfx_Nodes[mfxType]
    for i = 1, #nodes, 1 do
        local node = nodes[i]
        if node.pos == nil then goto continue end
        if from < 0 then from = i end
        local default_value_index = i - from + 1
        if default_value_index <= #prm.leaf then
            local default_value = prm.leaf[default_value_index].init
            local bytesize = Get_Byte_Size(node.valueByteSizeType)
            local bytes = Value_To_Bytes(default_value, bytesize)
            payload = ConcatTable(payload, bytes)
        end
        ::continue::
    end
    local idtemplate = Mfx_Ids[mfxType] .. "-" .. nodes[from].id
    local id = CreateId(idtemplate, part_id)
    local addr = Get_Adress(id)
    local sysex = Create_Sysex_Message_For_Payload(addr, payload, GetDeviceId())
    return sysex
end

local function getMfxData(part)
    local idx = mfxNumberPartMap[part]
    return Mfx_Table[idx]
end

local function mfxTypeChange(param, part, index)
    mfxNumberPartMap[part] = index
    local chgMsg = CreateSysexMessage(param.id, index)
    local defaultValues = get_set_mfx_default_values_sysex(Mfx_Types.PCMD, part, index)
    local msg = Concat(chgMsg, defaultValues)
    return msg
end

local function mfxName(part, mfxNr)
    local mfxData = getMfxData(part).leaf[mfxNr]
    if mfxData == nil then
        return HideParam
    end
    return mfxData.desc
end

local function mfxMin(part, mfxNr)
    local mfxData = getMfxData(part).leaf[mfxNr]
    if mfxData == nil then
        return 0
    end
    return 0
end

local function mfxMax(part, mfxNr)
    local mfxData = getMfxData(part).leaf[mfxNr]
    if mfxData == nil then
        return 127
    end
    return mfxData.max - mfxData.min
end

local function i7offsetValue(part, mfxNr)
    return function(guiValue)
        local mfxData = getMfxData(part).leaf[mfxNr]
        if mfxData == nil then
            return 0
        end
        return math.tointeger(mfxData.min + guiValue)
    end
end

local function guiOffsetValue(part, mfxNr)
    return function(i7Value)
        local mfxData = getMfxData(part).leaf[mfxNr]
        if mfxData == nil then
            return 0
        end
        return math.tointeger(i7Value - mfxData.min)
    end
end

local mfxCtrlIds = {
    { src = "RFRF_MFX_CTRL1_SRC", sens = "RFRF_MFX_CTRL1_SENS" },
    { src = "RFRF_MFX_CTRL2_SRC", sens = "RFRF_MFX_CTRL2_SENS" },
    { src = "RFRF_MFX_CTRL3_SRC", sens = "RFRF_MFX_CTRL3_SENS" },
    { src = "RFRF_MFX_CTRL4_SRC", sens = "RFRF_MFX_CTRL4_SENS" },
}

function CreateMfxPcmdSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " PCM-D MFX"

        -- Common section: MFX Type, Sends, 32 dynamic MFX params
        local commonKey     = prefix .. " Common"
        local commonSection = {
            name               = commonKey,
            hideFromPalette    = true,
            getReceiveValueSysex = function()
                return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_RHY-_RF")
            end,
            grp = {
                {
                    name   = "MFX Common",
                    params = {
                        {type="select", id=idTmpl("RFRF_MFX_TYPE"),      name=get("MFX Type"),              default=0,   options=mfxTypes},
                        {type="range",  id=idTmpl("RFRF_MFX_DRY_SEND"),  name=get("MFX Dry Send Level"),    default=127, min=get(0), max=get(127)},
                        {type="range",  id=idTmpl("RFRF_MFX_CHO_SEND"),  name=get("MFX Chorus Send Level"), default=0,   min=get(0), max=get(127)},
                        {type="range",  id=idTmpl("RFRF_MFX_REV_SEND"),  name=get("MFX Reverb Send Level"), default=0,   min=get(0), max=get(127)},
                    }
                },
                {
                    name   = "MFX",
                    params = {}
                }
            }
        }
        main[commonKey] = commonSection
        local subCommon = commonSection.grp[1]
        local subMfx    = commonSection.grp[2]

        local function mfxChangedHandler(leafNode, response)
            if not IsIdForPart(leafNode.fullid, partNr) then
                return nil
            end
            if leafNode.node.id == "RFRF_MFX_TYPE" then
                mfxNumberPartMap[partNr] = Bytes_To_Value(response.payload)
            end
            return nil
        end
        AddReceiveHandler(mfxChangedHandler)

        for _, param in ipairs(subCommon.params) do
            local isMfxChangeType = param.id == idTmpl("RFRF_MFX_TYPE")
            param.id = CreateId(param.id, partNr)
            if isMfxChangeType then
                param.setValue = function(value)
                    return mfxTypeChange(param, partNr, value)
                end
            else
                param = ParameterSetValueWrapper(param)
            end
        end
        for mfxNr = 0, 31, 1 do
            local id = idTmpl("RFRF_MFX_PRM" .. tostring(mfxNr + 1))
            id = CreateId(id, partNr)
            local p = {type="range", id=id, default=0}
            p = ParameterSetValueWrapper(p)
            p.toI7Value  = i7offsetValue(partNr, mfxNr)
            p.toGuiValue = guiOffsetValue(partNr, mfxNr)
            p.name = function() return mfxName(partNr, mfxNr) end
            p.min  = function() return mfxMin(partNr, mfxNr) end
            p.max  = function() return mfxMax(partNr, mfxNr) end
            table.insert(subMfx.params, p)
        end

        -- MFX Control 1-4 sections (one tab each)
        local tabs = {
            {label = "Common", keys = { {key = commonKey} }},
        }
        for ctrlNr = 1, 4, 1 do
            local ctrlKey = prefix .. " Control " .. ctrlNr
            local ids     = mfxCtrlIds[ctrlNr]
            local ctrlSection = {
                name            = ctrlKey,
                hideFromPalette = true,
                params          = {
                    {type="range", name=get("MFX Control " .. ctrlNr .. " Source"), id=idTmpl(ids.src),  default=0,  min=get(0), max=get(101)},
                    {type="range", name=get("MFX Control " .. ctrlNr .. " Sens"),   id=idTmpl(ids.sens), default=64, min=get(1), max=get(127)},
                }
            }
            for _, param in ipairs(ctrlSection.params) do
                param.id = CreateId(param.id, partNr)
                param = ParameterSetValueWrapper(param)
            end
            main[ctrlKey] = ctrlSection
            table.insert(tabs, {label = "Control " .. ctrlNr, keys = { {key = ctrlKey} }})
        end

        main[prefix] = {name = prefix, tabs = tabs}
    end
end
