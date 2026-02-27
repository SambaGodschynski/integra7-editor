require "math"
require "_mfx"
require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local function idTmpl(mfxId)
    return "PRM-_FPARTxxx-_SHPAT-_SHPF-" .. mfxId
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
    local defaultValues = get_set_mfx_default_values_sysex(Mfx_Types.SNS, part, index)
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

local mfxTemplate = {
    name = "SN-S Mfx",
    getReceiveValueSysex = nil,
    grp = {
        {
            name = "Mfx Common",
            params = {
                {type="select", id=idTmpl("SHPF_MFX_TYPE"),      name=get("MFX Type"),              default=0,   options=mfxTypes},
                {type="range",  id=idTmpl("SHPF_MFX_DRY_SEND"),  name=get("MFX Dry Send Level"),    default=127, min=get(0), max=get(127)},
                {type="range",  id=idTmpl("SHPF_MFX_CHO_SEND"),  name=get("MFX Chorus Send Level"), default=0,   min=get(0), max=get(127)},
                {type="range",  id=idTmpl("SHPF_MFX_REV_SEND"),  name=get("MFX Reverb Send Level"), default=0,   min=get(0), max=get(127)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL1_SRC"), name=get("MFX Control 1 Source"),  default=0,   min=get(0), max=get(101)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL1_SENS"),name=get("MFX Control 1 Sens"),    default=64,  min=get(1), max=get(127)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL2_SRC"), name=get("MFX Control 2 Source"),  default=0,   min=get(0), max=get(101)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL2_SENS"),name=get("MFX Control 2 Sens"),    default=64,  min=get(1), max=get(127)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL3_SRC"), name=get("MFX Control 3 Source"),  default=0,   min=get(0), max=get(101)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL3_SENS"),name=get("MFX Control 3 Sens"),    default=64,  min=get(1), max=get(127)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL4_SRC"), name=get("MFX Control 4 Source"),  default=0,   min=get(0), max=get(101)},
                {type="range",  id=idTmpl("SHPF_MFX_CTRL4_SENS"),name=get("MFX Control 4 Sens"),    default=64,  min=get(1), max=get(127)},
            }
        },
        {
            name = "Mfx",
            params = {}
        }
    }
}

function CreateMfxSnsSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " SN-S MFX"
        local mfxData = DeepCopy(mfxTemplate)
        mfxData.name = k
        mfxData.getReceiveValueSysex = function()
            return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_SHPAT-_SHPF")
        end
        main[k] = mfxData
        local subCommon = mfxData.grp[1]
        local subMfx    = mfxData.grp[2]

        local function mfxChangedHandler(leafNode, response)
            if not IsIdForPart(leafNode.fullid, partNr) then
                return nil
            end
            if leafNode.node.id == "SHPF_MFX_TYPE" then
                local mfxType = Bytes_To_Value(response.payload)
                mfxNumberPartMap[partNr] = mfxType
            end
            return nil
        end
        AddReceiveHandler(mfxChangedHandler)

        for _, param in ipairs(subCommon.params) do
            local isMfxChangeType = param.id == idTmpl("SHPF_MFX_TYPE")
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
            local id = idTmpl("SHPF_MFX_PRM" .. tostring(mfxNr + 1))
            id = CreateId(id, partNr)
            local p = {type="range", id=id, default=0}
            p = ParameterSetValueWrapper(p)
            p.toI7Value  = i7offsetValue(partNr, mfxNr)
            p.toGuiValue = guiOffsetValue(partNr, mfxNr)
            p.name = function()
                return mfxName(partNr, mfxNr)
            end
            p.min = function()
                return mfxMin(partNr, mfxNr)
            end
            p.max = function()
                return mfxMax(partNr, mfxNr)
            end
            table.insert(subMfx.params, p)
        end
    end
end
