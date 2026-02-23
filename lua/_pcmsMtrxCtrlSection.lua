require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

-- Source options: OFF, CC01-CC95 (indices 1-95), BEND, AFT, CTRL1-4,
-- VELOCITY, KEYFOLLOW, TEMPO, LFO1, LFO2, PIT-ENV, TVF-ENV, TVA-ENV (max=109)
local CtrlSrc = {[0]="OFF"}
for i = 1, 95 do
    CtrlSrc[i] = string.format("CC%02d", i)
end
CtrlSrc[96]  = "BEND"
CtrlSrc[97]  = "AFT"
CtrlSrc[98]  = "CTRL1"
CtrlSrc[99]  = "CTRL2"
CtrlSrc[100] = "CTRL3"
CtrlSrc[101] = "CTRL4"
CtrlSrc[102] = "VELOCITY"
CtrlSrc[103] = "KEYFOLLOW"
CtrlSrc[104] = "TEMPO"
CtrlSrc[105] = "LFO1"
CtrlSrc[106] = "LFO2"
CtrlSrc[107] = "PIT-ENV"
CtrlSrc[108] = "TVF-ENV"
CtrlSrc[109] = "TVA-ENV"

local CtrlDst = {
    [0]="OFF",       [1]="PCH",       [2]="CUT",       [3]="RES",
    [4]="LEV",       [5]="PAN",       [6]="DRY",       [7]="CHO",
    [8]="REV",       [9]="PIT-LFO1",  [10]="PIT-LFO2",
    [11]="TVF-LFO1", [12]="TVF-LFO2", [13]="TVA-LFO1", [14]="TVA-LFO2",
    [15]="PAN-LFO1", [16]="PAN-LFO2",
    [17]="LFO1-RATE",[18]="LFO2-RATE",
    [19]="PIT-ATK",  [20]="PIT-DCY",  [21]="PIT-REL",
    [22]="TVF-ATK",  [23]="TVF-DCY",  [24]="TVF-REL",
    [25]="TVA-ATK",  [26]="TVA-DCY",  [27]="TVA-REL",
    [28]="PMT",      [29]="FXM",
    [30]="---",      [31]="---",      [32]="---",      [33]="---",
}

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end

function CreatePcmsMtrxCtrlSections(main)
    for partNr = 1, 16, 1 do
        local function pc(name)
            return "PRM-_FPART"..partNr.."-_PAT-_PC-RFPC_"..name
        end
        local receiveFunc = function()
            return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PC")
        end

        for ctrlNr = 1, 4, 1 do
            local kMtrx = "Part " .. string.format("%02d", partNr) .. " PCM-S MTRX CTRL " .. ctrlNr
            local cn     = "CTRL" .. ctrlNr
            local params = {
                {type="select", id=pc(cn.."_SRC"),   name=get("Source"), default=0,  options=CtrlSrc},
                {type="select", id=pc(cn.."_DST1"),  name=get("Dest 1"), default=0,  options=CtrlDst},
                {type="range",  id=pc(cn.."_SENS1"), name=get("Sens 1"), default=0,  min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="select", id=pc(cn.."_DST2"),  name=get("Dest 2"), default=0,  options=CtrlDst},
                {type="range",  id=pc(cn.."_SENS2"), name=get("Sens 2"), default=0,  min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="select", id=pc(cn.."_DST3"),  name=get("Dest 3"), default=0,  options=CtrlDst},
                {type="range",  id=pc(cn.."_SENS3"), name=get("Sens 3"), default=0,  min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="select", id=pc(cn.."_DST4"),  name=get("Dest 4"), default=0,  options=CtrlDst},
                {type="range",  id=pc(cn.."_SENS4"), name=get("Sens 4"), default=0,  min=get(-63), max=get(63), format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
            }
            for _, param in ipairs(params) do ParameterSetValueWrapper(param) end
            main[kMtrx] = {
                name                 = "Part " .. partNr .. " PCM-S MTRX CTRL " .. ctrlNr,
                params               = params,
                getReceiveValueSysex = receiveFunc,
            }
        end
    end
end
