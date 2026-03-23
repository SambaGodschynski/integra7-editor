require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local sc = "PRM-_SYS-_SC-"

-- Control Source options: OFF, CC01-CC31, CC33-CC95 (skip CC32), PITCH BEND, AFTERTOUCH
local CtrlSrcOptions = {[0]="OFF"}
do
    local ccNames = {
        [1]="MODULATION",  [2]="BREATH",      [4]="FOOT TYPE",   [5]="PORTA TIME",
        [6]="DATA ENTRY",  [7]="VOLUME",       [8]="BALANCE",     [10]="PANPOT",
        [11]="EXPRESSION", [16]="GENERAL-1",   [17]="GENERAL-2",  [18]="GENERAL-3",
        [19]="GENERAL-4",  [38]="DATA ENTRY",  [64]="HOLD-1",     [65]="PORTAMENTO",
        [66]="SOSTENUTO",  [67]="SOFT",        [68]="LEGATO SW",  [69]="HOLD-2",
        [71]="RESONANCE",  [72]="RELEASE TM",  [73]="ATTACK TM",  [74]="CUTOFF",
        [75]="DECAY TIME", [76]="VIB RATE",    [77]="VIB DEPTH",  [78]="VIB DELAY",
        [80]="GENERAL-5",  [81]="GENERAL-6",   [82]="GENERAL-7",  [83]="GENERAL-8",
        [84]="PORTA CTRL", [91]="REVERB",      [92]="TREMOLO",    [93]="CHORUS",
        [94]="CELESTE",    [95]="PHASER",
    }
    for i = 1, 31 do
        local n = ccNames[i]
        CtrlSrcOptions[i] = n and ("CC"..string.format("%02d",i)..":"..n) or ("CC"..string.format("%02d",i))
    end
    for cc = 33, 95 do
        local idx = cc - 1
        local n = ccNames[cc]
        CtrlSrcOptions[idx] = n and ("CC"..tostring(cc)..":"..n) or ("CC"..tostring(cc))
    end
    CtrlSrcOptions[95] = "PITCH BEND"
    CtrlSrcOptions[96] = "AFTERTOUCH"
    CtrlSrcOptions[97] = "---##sys97"
end

local PrfCtrlCh = {}
for i = 1, 16 do PrfCtrlCh[i-1] = tostring(i) end
PrfCtrlCh[16] = "OFF"

local function p(param)
    return ParameterSetValueWrapper(param)
end

function CreateSystemSections(main)
    local soundParams = {
        p({type="range",  name=get("Master Level"),     id=sc.."NESC_LEVEL",     default=127, min=get(0),   max=get(127)}),
        p({type="range",  name=get("Master Tune"),      id=sc.."NESC_TUNE",      default=440.0, min=get(415.3), max=get(466.2), format="%.1f",
            toI7Value  = function(hz)  return math.tointeger(math.log(hz / 440.0) / math.log(2) * 12000.0 + 1024) end,
            toGuiValue = function(i7)  return 440.0 * (2 ^ ((i7 - 1024) / 12000.0)) end,
        }),
        p({type="range",  name=get("Master Key Shift"), id=sc.."NESC_KEY_SHIFT", default=0,   min=get(-24), max=get(24),
            toI7Value  = function(gui) return math.tointeger(gui + 64) end,
            toGuiValue = function(i7)  return math.tointeger(i7 - 64)  end,
        }),
    }

    local syncTempoParams = {
        p({type="select", name=get("Clock Source"),         id=sc.."NESC_CLK_SRC",  default=0, options={[0]="MIDI", [1]="USB"}}),
        p({type="range",  name=get("System Tempo"),         id=sc.."NESC_TEMPO",    default=120, min=get(20), max=get(250)}),
        p({type="select", name=get("Tempo Assign Source"),   id=sc.."NESC_OR_TEMPO", default=0, options={[0]="System", [1]="Studio Set"}}),
    }

    local midiParams = {
        p({type="select", name=get("Studio Set Control Ch"), id=sc.."NESC_PRF_CTRL_CH", default=15, options=PrfCtrlCh}),
        p({type="toggle", name=get("Rx Program Change"),     id=sc.."NESC_RX_PC",        default=1}),
        p({type="toggle", name=get("Rx Bank Select"),        id=sc.."NESC_RX_BS",        default=1}),
    }

    local motionalParams = {
        p({type="select", name=get("2CH Out Mode"),               id=sc.."NESC_RSS_OUTMODE",      default=0, options={[0]="SPEAKER", [1]="PHONES"}}),
        p({type="toggle", name=get("5.1CH Center Speaker Switch"),id=sc.."NESC_RSS_CENTERSP_SW",  default=0}),
        p({type="toggle", name=get("5.1CH Subwoofer Switch"),     id=sc.."NESC_RSS_WOOFER_SW",    default=0}),
    }

    local controlParams = {
        p({type="select", name=get("System Control 1 Source"), id=sc.."NESC_CTRL1_SRC", default=0, options=CtrlSrcOptions}),
        p({type="select", name=get("System Control 2 Source"), id=sc.."NESC_CTRL2_SRC", default=0, options=CtrlSrcOptions}),
        p({type="select", name=get("System Control 3 Source"), id=sc.."NESC_CTRL3_SRC", default=0, options=CtrlSrcOptions}),
        p({type="select", name=get("System Control 4 Source"), id=sc.."NESC_CTRL4_SRC", default=0, options=CtrlSrcOptions}),
        p({type="select", name=get("Control Source Select"),   id=sc.."NESC_CTRL_SRC",  default=1, options={[0]="SYSTEM", [1]="STUDIO SET"}}),
    }

    main["System"] = {
        name = "System",
        accordion = true,
        params = {},
        getReceiveValueSysex = function()
            return CreateReceiveMessageForBranch("PRM-_SYS-_SC")
        end,
        grp = {
            {name = "Sound",             params = soundParams},
            {name = "Sync/Tempo",        params = syncTempoParams},
            {name = "MIDI",              params = midiParams},
            {name = "Motional Surround", params = motionalParams},
            {name = "Control",           params = controlParams},
        },
    }
end
