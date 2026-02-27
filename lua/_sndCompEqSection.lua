require "math"
require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local CompAttack = {
    [0]="0.05ms",  [1]="0.06ms",  [2]="0.07ms",  [3]="0.08ms",  [4]="0.09ms",
    [5]="0.1ms",   [6]="0.2ms",   [7]="0.3ms",   [8]="0.4ms",   [9]="0.5ms",
    [10]="0.6ms",  [11]="0.7ms",  [12]="0.8ms",  [13]="0.9ms",  [14]="1.0ms",
    [15]="2.0ms",  [16]="3.0ms",  [17]="4.0ms",  [18]="5.0ms",  [19]="6.0ms",
    [20]="7.0ms",  [21]="8.0ms",  [22]="9.0ms",  [23]="10.0ms", [24]="15.0ms",
    [25]="20.0ms", [26]="25.0ms", [27]="30.0ms", [28]="35.0ms", [29]="40.0ms",
    [30]="45.0ms", [31]="50.0ms",
}

local CompRelease = {
    [0]="0.05ms",  [1]="0.07ms",  [2]="0.1ms",   [3]="0.5ms",   [4]="1ms",
    [5]="5ms",     [6]="10ms",    [7]="17ms",    [8]="25ms",    [9]="50ms",
    [10]="75ms",   [11]="100ms",  [12]="200ms",  [13]="300ms",  [14]="400ms",
    [15]="500ms",  [16]="600ms",  [17]="700ms",  [18]="800ms",  [19]="900ms",
    [20]="1000ms", [21]="1200ms", [22]="1500ms", [23]="2000ms",
}

local CompRatio = {
    [0]="1:1",   [1]="2:1",   [2]="3:1",   [3]="4:1",   [4]="5:1",
    [5]="6:1",   [6]="7:1",   [7]="8:1",   [8]="9:1",   [9]="10:1",
    [10]="20:1", [11]="30:1", [12]="40:1", [13]="50:1", [14]="60:1",
    [15]="70:1", [16]="80:1", [17]="90:1", [18]="100:1",[19]="inf:1",
}

local EqLowFreq  = {[0]="200 Hz", [1]="400 Hz"}
local EqMidFreq  = {
    [0]="200 Hz",  [1]="250 Hz",  [2]="315 Hz",  [3]="400 Hz",  [4]="500 Hz",
    [5]="630 Hz",  [6]="800 Hz",  [7]="1000 Hz", [8]="1250 Hz", [9]="1600 Hz",
    [10]="2000 Hz",[11]="2500 Hz",[12]="3150 Hz",[13]="4000 Hz",[14]="5000 Hz",
    [15]="6300 Hz",[16]="8000 Hz",
}
local EqMidQ     = {[0]="0.5", [1]="1.0", [2]="2.0", [3]="4.0", [4]="8.0"}
local EqHighFreq = {[0]="2000 Hz", [1]="4000 Hz", [2]="8000 Hz"}

local function i7eqgain(gui) return math.tointeger(gui + 15) end
local function guieqgain(i7) return math.tointeger(i7 - 15) end

function CreateSndCompEqSections(main)
    for partNr = 1, 16, 1 do
        local pn = string.format("%02d", partNr)
        for ch = 1, 6, 1 do
            local k = "Part " .. pn .. " SN-D CompEq " .. ch
            local n = tostring(ch)

            local function idKcq(name)
                return "PRM-_FPART" .. partNr .. "-_KIT-_KCQ-SDKCQ_" .. name
            end

            local params = {
                {type="toggle", id=idKcq("COMP" .. n .. "_SW"),        name=get("Comp Switch"),   default=0,  min=get(0),   max=get(1)},
                {type="select", id=idKcq("COMP" .. n .. "_ATTACK"),    name=get("Attack Time"),   default=0,  options=CompAttack},
                {type="select", id=idKcq("COMP" .. n .. "_RELEASE"),   name=get("Release Time"),  default=0,  options=CompRelease},
                {type="range",  id=idKcq("COMP" .. n .. "_THRESHOLD"), name=get("Threshold"),     default=0,  min=get(0),   max=get(127)},
                {type="select", id=idKcq("COMP" .. n .. "_RATIO"),     name=get("Ratio"),         default=0,  options=CompRatio},
                {type="range",  id=idKcq("COMP" .. n .. "_POSTGAIN"),  name=get("Output Gain"),   default=0,  min=get(0),   max=get(24)},
                {type="toggle", id=idKcq("EQ"   .. n .. "_SW"),        name=get("EQ Switch"),     default=0,  min=get(0),   max=get(1)},
                {type="select", id=idKcq("EQ"   .. n .. "_LOWFREQ"),   name=get("Low Freq"),      default=0,  options=EqLowFreq},
                {type="range",  id=idKcq("EQ"   .. n .. "_LOWGAIN"),   name=get("Low Gain"),      default=0,  min=get(-15), max=get(15), format="%+.0f dB", toI7Value=i7eqgain, toGuiValue=guieqgain},
                {type="select", id=idKcq("EQ"   .. n .. "_MIDFREQ"),   name=get("Mid Freq"),      default=0,  options=EqMidFreq},
                {type="range",  id=idKcq("EQ"   .. n .. "_MIDGAIN"),   name=get("Mid Gain"),      default=0,  min=get(-15), max=get(15), format="%+.0f dB", toI7Value=i7eqgain, toGuiValue=guieqgain},
                {type="select", id=idKcq("EQ"   .. n .. "_MIDQ"),      name=get("Mid Q"),         default=0,  options=EqMidQ},
                {type="select", id=idKcq("EQ"   .. n .. "_HIGHFREQ"),  name=get("High Freq"),     default=0,  options=EqHighFreq},
                {type="range",  id=idKcq("EQ"   .. n .. "_HIGHGAIN"),  name=get("High Gain"),     default=0,  min=get(-15), max=get(15), format="%+.0f dB", toI7Value=i7eqgain, toGuiValue=guieqgain},
            }

            for _, param in ipairs(params) do
                ParameterSetValueWrapper(param)
            end

            main[k] = {
                name = k,
                params = params,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART" .. partNr .. "-_KIT-_KCQ")
                end,
            }
        end
    end
end
