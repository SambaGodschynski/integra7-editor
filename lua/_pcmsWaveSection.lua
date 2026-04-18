require "_sysex"
require "_com"
require "_model"
require "_waveData"
require "_srxWaveData"

local get = GetWrapper

local WaveGroup = {
    [0]="Internal (XV-5080)",
    [1]="SRX-01", [2]="SRX-02", [3]="SRX-03", [4]="SRX-04",
    [5]="SRX-05", [6]="SRX-06", [7]="SRX-07", [8]="SRX-08",
    [9]="SRX-09", [10]="SRX-10", [11]="SRX-11", [12]="SRX-12",
}
local WaveGain  = {[0]="-6dB", [1]="0dB", [2]="+6dB", [3]="+12dB"}
local FxmColor  = {[0]="1", [1]="2", [2]="3", [3]="4"}
local DelayMode = {[0]="Normal", [1]="Hold", [2]="Key-Off Normal", [3]="Key-Off Hold"}

-- per-partial GID state: currentGid[partNr][partialNr]
local currentGid = {}

local function waveOptionsFor(partNr, partialNr)
    local gid = (currentGid[partNr] or {})[partialNr] or 0
    if gid >= 1 and gid <= 12 and SrxWaveNames[gid] then
        return SrxWaveNames[gid]
    end
    return WaveNames
end

function CreatePcmsWaveSections(main)
    for partNr = 1, 16, 1 do
        currentGid[partNr] = {}
        for partialNr = 1, 4, 1 do
            local k = "Part " .. string.format("%02d", partNr) .. " PCM-S Wave Partial " .. partialNr

            local function p(name)
                return "PRM-_FPART" .. partNr .. "-_PAT-_PT" .. partialNr .. "-RFPT_" .. name
            end

            -- WAV_GID: wrap setValue to also update local GID state
            local gidParam = {type="select", id=p("WAV_GID"), name=get("Wave Group"), default=0, options=WaveGroup}
            gidParam = ParameterSetValueWrapper(gidParam)
            local origGidSetValue = gidParam.setValue
            gidParam.setValue = function(v)
                currentGid[partNr][partialNr] = v
                return origGidSetValue(v)
            end

            -- WAV_NUML / WAV_NUMR: options depend on current GID
            local pNr, partNr2, partialNr2 = partNr, partNr, partialNr
            local waveNumL = {type="select", id=p("WAV_NUML"), name=get("Wave No. L (Mono)"), default=1,
                options=function() return waveOptionsFor(partNr2, partialNr2) end}
            local waveNumR = {type="select", id=p("WAV_NUMR"), name=get("Wave No. R"),         default=0,
                options=function() return waveOptionsFor(partNr2, partialNr2) end}

            -- Wrap remaining params (gidParam/waveNumL/waveNumR already handled above)
            local toWrap = {
                waveNumL,
                waveNumR,
                {type="select", id=p("WAV_GAIN"),   name=get("Wave Gain"),          default=1,  options=WaveGain},
                {type="toggle", id=p("TEMPO_SYNC"), name=get("Tempo Sync"),         default=0,  min=get(0), max=get(1)},
                {type="toggle", id=p("FXM_SW"),     name=get("FXM Switch"),         default=0,  min=get(0), max=get(1)},
                {type="select", id=p("FXM_COLOR"),  name=get("FXM Color"),          default=0,  options=FxmColor},
                {type="range",  id=p("FXM_DEPTH"),  name=get("FXM Depth"),          default=0,  min=get(0), max=get(16), format="%.0f"},
                {type="select", id=p("DELAY_MODE"), name=get("Partial Delay Mode"), default=0,  options=DelayMode},
                {type="range",  id=p("DELAY_TIME"), name=get("Partial Delay Time"), default=0,  min=get(0), max=get(149), format="%.0f"},
            }
            for _, param in ipairs(toWrap) do
                param = ParameterSetValueWrapper(param)
            end

            local params = {gidParam}
            for _, param in ipairs(toWrap) do
                table.insert(params, param)
            end

            main[k] = {
                name = "Part " .. string.format("%02d", partNr) .. " PCM-S Wave Partial " .. partialNr,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch(
                        "PRM-_FPART" .. partNr .. "-_PAT-_PT" .. partialNr
                    )
                end,
            }
        end
    end
end
