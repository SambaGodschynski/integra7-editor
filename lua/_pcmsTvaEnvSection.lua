require "_sysex"
require "_com"
require "_model"

local get = GetWrapper

local function i7offset(offset)
    return function(gui) return math.tointeger(offset + gui) end
end
local function guiOffset(offset)
    return function(i7) return math.tointeger(i7 - offset) end
end
-- ×10 scale with centre at 64  (AENV_TKF: -100..+100 step 10)
local function i7scale10(gui) return math.tointeger(gui / 10 + 64) end
local function gui10scale(i7) return math.tointeger((i7 - 64) * 10) end

-- Virtual anchor param: always 0, never sends MIDI.
-- Used as the implicit L0 / L4 endpoints of the TVA envelope (silence).
local function makeAnchor(id)
    local p = {type="range", id=id, name=get(HideParam),
               default=0, min=get(0), max=get(127), format="%.0f"}
    p.setValue = function(_) return {} end  -- no-op: L0/L4 are not Integra-7 params
    return p
end

function CreatePcmsTvaEnvSections(main)
    for partNr = 1, 16, 1 do
        for partialNr = 1, 4, 1 do
            local kPartial = "Part " .. string.format("%02d", partNr) .. " PCM-S TVA Env Partial " .. partialNr
            local function p(name)
                return "PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr.."-RFPT_"..name
            end

            -- Unique IDs for the virtual anchor params (not in the Integra-7 model)
            local idL0 = kPartial .. "-AENV_L0_ANCHOR"
            local idL4 = kPartial .. "-AENV_L4_ANCHOR"

            local partialParams = {
                -- Interactive envelope widget
                -- TVA envelope: silence(L0=0) → T1 → L1 → T2 → L2 → T3 → L3 → T4 → silence(L4=0)
                {type="envelope", id=kPartial.."-ENV", name=get("TVA Envelope"),
                 levelIds={idL0, p("AENV_L1"), p("AENV_L2"), p("AENV_L3"), idL4},
                 timeIds ={p("AENV_T1"), p("AENV_T2"), p("AENV_T3"), p("AENV_T4")},
                 sustainSegment=true},

                -- Visible knobs
                {type="range", id=p("AENV_T1_VSENS"), name=get("TVA Env T1 V-Sns"), default=0, min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("AENV_T4_VSENS"), name=get("TVA Env T4 V-Sns"), default=0, min=get(-63), max=get(63),  format="%+.0f", toI7Value=i7offset(64), toGuiValue=guiOffset(64)},
                {type="range", id=p("AENV_TKF"),      name=get("TVA Env Time KF"),   default=0, min=get(-100),max=get(100), format="%+.0f", toI7Value=i7scale10,    toGuiValue=gui10scale},

                -- Hidden real L/T params
                {type="range", id=p("AENV_T1"), name=get(HideParam), default=0,   min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("AENV_T2"), name=get(HideParam), default=10,  min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("AENV_T3"), name=get(HideParam), default=10,  min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("AENV_T4"), name=get(HideParam), default=10,  min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("AENV_L1"), name=get(HideParam), default=127, min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("AENV_L2"), name=get(HideParam), default=127, min=get(0), max=get(127), format="%.0f"},
                {type="range", id=p("AENV_L3"), name=get(HideParam), default=127, min=get(0), max=get(127), format="%.0f"},

                -- Virtual anchor params (always 0, no MIDI send)
                makeAnchor(idL0),
                makeAnchor(idL4),
            }
            for _, param in ipairs(partialParams) do
                if param.type ~= "envelope" and param.setValue == nil then
                    ParameterSetValueWrapper(param)
                end
            end
            main[kPartial] = {
                name   = "Part " .. string.format("%02d", partNr) .. " PCM-S TVA Env Partial " .. partialNr,
                params = partialParams,
                getReceiveValueSysex = function()
                    return CreateReceiveMessageForBranch("PRM-_FPART"..partNr.."-_PAT-_PT"..partialNr)
                end,
            }
        end
    end
end
