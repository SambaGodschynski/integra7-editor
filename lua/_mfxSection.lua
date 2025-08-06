require "math"
require "_mfx"
require "_sysex"
require "_com"

local get = GetWrapper

local function idTmpl(mfxId)
    return "PRM-_FPARTxxx-_SNTONE-_SNTF-" .. mfxId
end

local mfxNumberPartMap = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

local mfxTypes = MapDict(Mfx_Table, function (type)
    return type.desc
end)

local function getMfxData(part)
    local idx = mfxNumberPartMap[part]
    return Mfx_Table[idx]
end

local function mfxTypeChange(part, index)
    mfxNumberPartMap[part] = index
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
    return function (guiValue)
        local mfxData = getMfxData(part).leaf[mfxNr]
        if mfxData == nil then
            return 0
        end
        return math.tointeger(mfxData.min + guiValue)
    end
end

local function guiOffsetValue(part, mfxNr)
    return function (i7Value)
        local mfxData = getMfxData(part).leaf[mfxNr]
        if mfxData == nil then
            return 0
        end
        return math.tointeger(mfxData.min - i7Value)
    end
end


local mfxTemplate = {
    name = "Mfx",
    getReceiveValueSysex = nil,
    sub = {
        {
            name="Mfx Common",
            params = {
                {type="select", id=idTmpl("SNTF_MFX_TYPE"), name=get("MFX Type"), default=0, options=mfxTypes},
                {type="range", name=get("MFX Dry Send Level"), id=idTmpl("SNTF_MFX_DRY_SEND"), default=127, min=get(0), max=get(127)},
                {type="range", name=get("MFX Chorus Send Level"), id=idTmpl("SNTF_MFX_CHO_SEND"), default=0, min=get(0), max=get(127)},
                {type="range", name=get("MFX Reverb Send Level"), id=idTmpl("SNTF_MFX_REV_SEND"), default=0, min=get(0), max=get(127)},
                {type="range", name=get("MFX Control 1 Source"), id=idTmpl("SNTF_MFX_CTRL1_SRC"), default=0, min=get(0), max=get(101)},
                {type="range", name=get("MFX Control 1 Sens"), id=idTmpl("SNTF_MFX_CTRL1_SENS"), default=64, min=get(1), max=get(127)},
                {type="range", name=get("MFX Control 2 Source"), id=idTmpl("SNTF_MFX_CTRL2_SRC"), default=0, min=get(0), max=get(101)},
                {type="range", name=get("MFX Control 2 Sens"), id=idTmpl("SNTF_MFX_CTRL2_SENS"), default=64, min=get(1), max=get(127)},
                {type="range", name=get("MFX Control 3 Source"), id=idTmpl("SNTF_MFX_CTRL3_SRC"), default=0, min=get(0), max=get(101)},
                {type="range", name=get("MFX Control 3 Sens"), id=idTmpl("SNTF_MFX_CTRL3_SENS"), default=64, min=get(1), max=get(127)},
                {type="range", name=get("MFX Control 4 Source"), id=idTmpl("SNTF_MFX_CTRL4_SRC"), default=0, min=get(0), max=get(101)},
                {type="range", name=get("MFX Control 4 Sens"), id=idTmpl("SNTF_MFX_CTRL4_SENS"), default=64, min=get(1), max=get(127)},
            }
        },
        {
            name="Mfx",
            params = {}
        }
    }
}


function CreateMfxSections(main)
    for partNr = 1, 16, 1 do
        local k = "Part " .. string.format("%02d", partNr) .. " MFX"
        local name = k
        local mfxData = DeepCopy(mfxTemplate)
        mfxData.name = name
        if partNr==1 then
            mfxData.isOpen = true
        end
        main[k] = mfxData
        local subCommon = mfxData.sub[1]
        local subMfx = mfxData.sub[2]
        for _, param in ipairs(subCommon.params) do
                local isMfxChangeType = param.id == idTmpl("SNTF_MFX_TYPE")
                param.id = CreateId(param.id, partNr)
                if isMfxChangeType then
                    param = ParameterSetValueWrapper(param, function (value)
                        mfxTypeChange(partNr, value)
                    end)
                else
                    param = ParameterSetValueWrapper(param)
                end
        end
        for mfxNr = 0, 31, 1 do
            local id = idTmpl("SNTF_MFX_PRM" .. tostring(mfxNr))
            id = CreateId(id, partNr)
            local p = {type="range", id=id, default=0}
            p = ParameterSetValueWrapper(p)
            p.toI7Value = i7offsetValue(partNr, mfxNr)
            p.toGuiValue = guiOffsetValue(partNr, mfxNr)
            p.name = function ()
                return mfxName(partNr, mfxNr)
            end
            p.min = function ()
                return mfxMin(partNr, mfxNr)
            end
            p.max = function ()
                return mfxMax(partNr, mfxNr)
            end
            table.insert(subMfx.params, p)
        end
    end
end