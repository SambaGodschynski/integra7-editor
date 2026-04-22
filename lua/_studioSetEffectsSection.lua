require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local fc   = "PRM-_PRF-_FC-"
local fh   = "PRM-_PRF-_FH-"
local fv   = "PRM-_PRF-_FV-"
local fmeq = "PRM-_PRF-_FMEQ-"

local CompEqAssign = {
    [0]="PART", [1]="A", [2]="B", [3]="C", [4]="D",
    [5]="1", [6]="2", [7]="3", [8]="4", [9]="5", [10]="6", [11]="7", [12]="8",
}

local ChorusType = {
    [0]="OFF", [1]="Chorus", [2]="Delay", [3]="GM2 Chorus",
}

local ChorusOutputAssign = {[0]="A", [1]="B", [2]="C", [3]="D"}
local ChorusOutputSelect = {[0]="MAIN", [1]="REV", [2]="MAIN+REV"}

local ReverbType = {
    [0]="OFF", [1]="Room 1", [2]="Room 2", [3]="Hall 1",
    [4]="Hall 2", [5]="Plate", [6]="GM2 Reverb",
}

local ReverbOutputAssign = {[0]="A", [1]="B", [2]="C", [3]="D"}

local function i7eqgain(gui) return math.tointeger(math.floor(gui + 15 + 0.5)) end
local function guieqgain(i7) return math.tointeger(math.floor(i7 - 15 + 0.5)) end

local function p(param)
    return ParameterSetValueWrapper(param)
end

function CreateStudioSetEffectsSections(main)
    local compEqParams = {}
    for i = 1, 6 do
        table.insert(compEqParams, p({
            type="select", name=get("Comp+EQ " .. i .. " Output Assign"),
            id=fc.."NEFC_COMPEQ"..i.."_ASSIGN", default=0, options=CompEqAssign,
        }))
    end

    local chorusParams = {
        p({type="toggle", name=get("Chorus Switch"),        id=fc.."NEFC_CHORUS_SW",        default=1}),
        p({type="select", name=get("Chorus Type"),          id=fh.."NEFH_CHO_TYPE",         default=1, options=ChorusType}),
        p({type="range",  name=get("Chorus Level"),         id=fh.."NEFH_CHO_LEVEL",        default=127, min=get(0), max=get(127)}),
        p({type="select", name=get("Chorus Output Assign"), id=fh.."NEFH_CHO_OUT_ASGN",     default=0, options=ChorusOutputAssign}),
        p({type="select", name=get("Chorus Output Select"), id=fh.."NEFH_CHO_OUT_SELECT",   default=0, options=ChorusOutputSelect}),
    }

    local reverbParams = {
        p({type="toggle", name=get("Reverb Switch"),        id=fc.."NEFC_REVERB_SW",        default=1}),
        p({type="select", name=get("Reverb Type"),          id=fv.."NEFV_REV_TYPE",         default=1, options=ReverbType}),
        p({type="range",  name=get("Reverb Level"),         id=fv.."NEFV_REV_LEVEL",        default=127, min=get(0), max=get(127)}),
        p({type="select", name=get("Reverb Output Assign"), id=fv.."NEFV_REV_OUT_ASGN",     default=0, options=ReverbOutputAssign}),
    }

    -- renderEq3Band picks params by type (toggle=SW, vslider×3=gains, range×4=freqs+Q).
    -- Order within each type group must be: LowGain, MidGain, HighGain / LowFreq, MidFreq, MidQ, HighFreq
    local masterEqParams = {
        p({type="toggle",  name=get("Master EQ Switch"), id=fc.."NEFC_MASTER_EQ_SW",     default=0}),
        p({type="vslider", name=get("Low Gain"),         id=fmeq.."NEFMEQ_EQ_LOWGAIN",  default=0, min=get(-15), max=get(15), toI7Value=i7eqgain, toGuiValue=guieqgain}),
        p({type="vslider", name=get("Mid Gain"),         id=fmeq.."NEFMEQ_EQ_MIDGAIN",  default=0, min=get(-15), max=get(15), toI7Value=i7eqgain, toGuiValue=guieqgain}),
        p({type="vslider", name=get("High Gain"),        id=fmeq.."NEFMEQ_EQ_HIGHGAIN", default=0, min=get(-15), max=get(15), toI7Value=i7eqgain, toGuiValue=guieqgain}),
        p({type="range",   name=get("Low Freq"),         id=fmeq.."NEFMEQ_EQ_LOWFREQ",  default=1, min=get(0), max=get(1)}),
        p({type="range",   name=get("Mid Freq"),         id=fmeq.."NEFMEQ_EQ_MIDFREQ",  default=7, min=get(0), max=get(16)}),
        p({type="range",   name=get("Mid Q"),            id=fmeq.."NEFMEQ_EQ_MIDQ",     default=0, min=get(0), max=get(4)}),
        p({type="range",   name=get("High Freq"),        id=fmeq.."NEFMEQ_EQ_HIGHFREQ", default=1, min=get(0), max=get(2)}),
    }

    main["Studio Set Effects"] = {
        name = "Studio Set Effects",
        accordion = true,
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            for _, branch in ipairs({"PRM-_PRF-_FC", "PRM-_PRF-_FH", "PRM-_PRF-_FV", "PRM-_PRF-_FMEQ"}) do
                for _, msg in ipairs(CreateReceiveMessageForBranch(branch)) do
                    table.insert(result, msg)
                end
            end
            return result
        end,
        grp = {
            {name = "Comp+EQ Output", params = compEqParams},
            {name = "Chorus",         params = chorusParams},
            {name = "Reverb",         params = reverbParams},
            {name = "Master EQ", layout = "eq3band", params = masterEqParams},
        },
    }
end
