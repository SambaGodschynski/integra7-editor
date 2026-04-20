require "_com"
require "_sysex"

local p   = ParameterSetValueWrapper
local get = GetWrapper

local frss = "PRM-_PRF-_FRSS-"
local fc   = "PRM-_PRF-_FC-"

local roomTypeOptions = {[0]="STUDIO", [1]="STUDIO L", [2]="HALL", [3]="HALL L"}
local outModeOptions  = {[0]="PHONES", [1]="SPEAKERS"}

local ctrlChOptions = {}
for i = 0, 15 do ctrlChOptions[i] = tostring(i + 1) end
ctrlChOptions[16] = "OFF"

local function makeCommonSection()
    local s = { name = "Common", params = {} }
    table.insert(s.params, p({type="toggle", id=frss.."NEFRSS_RSS_SW",        name=get("RSS SW"),         min=get(0), max=get(1),   default=0}))
    table.insert(s.params, p({type="select", id=frss.."NEFRSS_RSS_ROOM_TYPE", name=get("Room Type"),      min=get(0), max=get(3),   default=2, options=roomTypeOptions}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_REV_LEVEL",     name=get("Rev Level"),      min=get(0), max=get(127), default=80}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_REV_ROOMSIZE",  name=get("Room Size"),      min=get(0), max=get(2),   default=1}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_REV_TIME",      name=get("Rev Time"),       min=get(0), max=get(100), default=40}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_REV_DENSITY",   name=get("Rev Density"),    min=get(0), max=get(100), default=100}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_REV_HFDUMP",    name=get("HF Damp"),        min=get(0), max=get(100), default=50}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_SP4XTALKC",     name=get("Xtalk Cancel"),   min=get(0), max=get(100), default=50}))
    table.insert(s.params, p({type="toggle", id=fc.."NESC_RSS_CENTERSP_SW",   name=get("Center Speaker"), min=get(0), max=get(1),   default=0}))
    table.insert(s.params, p({type="toggle", id=fc.."NESC_RSS_WOOFER_SW",     name=get("Subwoofer"),      min=get(0), max=get(1),   default=0}))
    table.insert(s.params, p({type="select", id=fc.."NESC_RSS_OUTMODE",       name=get("Output Mode"),    min=get(0), max=get(1),   default=0, options=outModeOptions}))
    return s
end

local function makeExtInputSection()
    local s = { name = "Ext Input", params = {} }
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_AUX_IN_XPOS",    name=get("Ext X"),        min=get(0), max=get(127), default=64}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_AUX_IN_YPOS",    name=get("Ext Y"),        min=get(0), max=get(127), default=74}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_AUX_IN_WIDTH",   name=get("Ext Width"),    min=get(0), max=get(32),  default=10}))
    table.insert(s.params, p({type="range",  id=frss.."NEFRSS_AUX_IN_RVSEND",  name=get("Ext Rev Send"), min=get(0), max=get(127), default=40}))
    table.insert(s.params, p({type="select", id=frss.."NEFRSS_AUX_IN_CTRL_CH", name=get("Ctrl Channel"), min=get(0), max=get(16),  default=16, options=ctrlChOptions}))
    return s
end

-- Position section: 4 params per part in order X, Y, Width, RevSend
local function makePositionSection()
    local s = { name = "Position", layout = "rss_xy", params = {} }
    for i = 1, 16 do
        local fp = "PRM-_PRF-_FP"..i.."-"
        table.insert(s.params, p({type="range", id=fp.."NEFP_RSS_X1POS", name=get("Part "..i.." X"),       min=get(0), max=get(127), default=64}))
        table.insert(s.params, p({type="range", id=fp.."NEFP_RSS_Y1POS", name=get("Part "..i.." Y"),       min=get(0), max=get(127), default=74}))
        table.insert(s.params, p({type="range", id=fp.."NEFP_RSS_WIDTH",  name=get("Part "..i),             min=get(0), max=get(32),  default=10}))
        table.insert(s.params, p({type="range", id=fp.."NEFP_RSS_RVSEND", name=get("Part "..i),             min=get(0), max=get(127), default=40}))
    end
    return s
end

function CreateRssSections(main)
    local rss = {
        name = "Motional Surround",
        accordion = true,
        params = {},
        getReceiveValueSysex = function()
            local result = {}
            for _, msg in ipairs(CreateReceiveMessageForBranch("PRM-_PRF-_FRSS")) do
                table.insert(result, msg)
            end
            for _, suffix in ipairs({"NESC_RSS_CENTERSP_SW", "NESC_RSS_WOOFER_SW", "NESC_RSS_OUTMODE"}) do
                local msg = CreateReceiveMessageForLeafId(fc..suffix)
                if msg then table.insert(result, msg) end
            end
            for i = 1, 16 do
                local fp = "PRM-_PRF-_FP"..i.."-"
                for _, suffix in ipairs({"NEFP_RSS_X1POS", "NEFP_RSS_Y1POS", "NEFP_RSS_WIDTH", "NEFP_RSS_RVSEND"}) do
                    local msg = CreateReceiveMessageForLeafId(fp..suffix)
                    if msg then table.insert(result, msg) end
                end
            end
            return result
        end,
        grp = {}
    }
    table.insert(rss.grp, makeCommonSection())
    table.insert(rss.grp, makeExtInputSection())
    table.insert(rss.grp, makePositionSection())
    main["rss"] = rss
end
