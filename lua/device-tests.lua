-- Device test runner for the Integra-7 editor.
--
-- Run with:
--   i7ed --lua-main lua/device-tests.lua \
--        --midi-in  "INTEGRA-7" \
--        --midi-out "INTEGRA-7"
--
-- ── Config ────────────────────────────────────────────────────────────────────

local Config = {
    -- Which part to use for tone-specific tests (1–16).
    test_part = 1,

    -- Enable/disable entire groups.
    -- Turn off a group to skip it (e.g. when debugging a specific area).
    groups = {
        SNA       = true,   -- SN-A tone sections + SNA MFX
        SNS       = true,   -- SN-S tone sections + SNS MFX
        SND       = true,   -- SN-D tone sections + SND MFX
        PCMS      = true,   -- PCM Synth sections + PCMS MFX
        PCMD      = true,   -- PCM Drum sections  + PCMD MFX
        Parts     = true,   -- Studio Set part params (level, pan, EQ, …)
        System    = true,   -- System section
        StudioSet = true,   -- Studio Set Effects / RSS / Expansion
    },
}

-- ── Normal section setup (identical to main.lua) ─────────────────────────────

require "_com"
require "_model"
require "_sysex"
require "_snaSection"
require "_patchesSection"
require "_partsSection"
require "_mfxSection"
require "_mfxPcmSSection"
require "_pcmsCommonSection"
require "_pcmsWaveSection"
require "_pcmsPmtSection"
require "_pcmsPitchSection"
require "_pcmsPitchEnvSection"
require "_pcmsTvfSection"
require "_pcmsTvfEnvSection"
require "_pcmsTvaSection"
require "_pcmsTvaEnvSection"
require "_pcmsOutputSection"
require "_pcmsLfoSection"
require "_pcmsCtrlSection"
require "_pcmsMtrxCtrlSection"
require "_pcmsTabsSection"
require "_snsCommonSection"
require "_snsMiscSection"
require "_snsOscSection"
require "_snsPitchSection"
require "_snsFilterSection"
require "_snsAmpSection"
require "_snsLfoSection"
require "_snsMfxSection"
require "_snsTabsSection"
require "_sndCommonSection"
require "_sndInstSection"
require "_sndCompEqSection"
require "_sndMfxSection"
require "_sndTabsSection"
require "_pcmdCommonSection"
require "_pcmdPitchSection"
require "_pcmdCompEqSection"
require "_pcmdMfxSection"
require "_pcmdTabsSection"
require "_expansionSection"
require "_rssSection"
require "_studioSetEffectsSection"
require "_systemSection"

Main = {}
CreateSnaSections(Main)
CreatePatchesSections(Main)
CreatePartsSections(Main)
CreateMfxSections(Main)
CreateMfxPcmSSections(Main)
CreatePcmsCommonSections(Main)
CreatePcmsWaveSections(Main)
CreatePcmsPmtSections(Main)
CreatePcmsPitchSections(Main)
CreatePcmsPitchEnvSections(Main)
CreatePcmsTvfSections(Main)
CreatePcmsTvfEnvSections(Main)
CreatePcmsTvaSections(Main)
CreatePcmsTvaEnvSections(Main)
CreatePcmsOutputSections(Main)
CreatePcmsLfoSections(Main)
CreatePcmsCtrlSections(Main)
CreatePcmsMtrxCtrlSections(Main)
CreatePcmsTabSections(Main)
CreateSnsCommonSections(Main)
CreateSnsMiscSections(Main)
CreateSnsOscSections(Main)
CreateSnsPitchSections(Main)
CreateSnsFilterSections(Main)
CreateSnsAmpSections(Main)
CreateSnsLfoSections(Main)
CreateMfxSnsSections(Main)
CreateSnsTabSections(Main)
CreateSndCommonSections(Main)
CreateSndInstSections(Main)
CreateSndCompEqSections(Main)
CreateMfxSndSections(Main)
CreateSndTabSections(Main)
CreatePcmdCommonSections(Main)
CreatePcmdPitchSections(Main)
CreatePcmdCompEqSections(Main)
CreateMfxPcmdSections(Main)
CreatePcmdTabSections(Main)
CreateExpansionSection(Main)
CreateRssSections(Main)
CreateStudioSetEffectsSections(Main)
CreateSystemSections(Main)

-- ── Test framework ────────────────────────────────────────────────────────────

local groups   = {}   -- ordered list of {name, coro}
local current  = nil  -- currently running coroutine
local passed   = 0
local failed   = 0
local skipped  = 0
local done     = false

local function log(msg)
    io.write(msg .. "\n")
    io.flush()
end

local function assert_eq(got, expected, desc)
    if got == expected then
        passed = passed + 1
    else
        failed = failed + 1
        log(string.format("  FAIL  %s\n        expected %s, got %s",
            desc, tostring(expected), tostring(got)))
    end
end

-- Send RQ1 for param, wait for response, return current GUI value.
local function request_param(id)
    RequestParam(id)
    repeat coroutine.yield() until not IsReceiving()
    return GetParam(id)
end

-- Set tone type for test_part.
-- idx: 1=SNA, 2=SNS, 3=SND, 4=PCMS, 5=PCMD
local function set_tone_type(idx)
    local id = "PRM-_PRF-_FP" .. Config.test_part .. "-NEFP_TYPE_DUMMY"
    SetParam(id, idx)
    coroutine.yield()  -- give device a moment
end

-- ── Param skip logic ──────────────────────────────────────────────────────────

local SKIP_TYPES = {
    envelope=true, steplfo=true, action=true,
    savesysex=true, loadsysex=true, solotoggle=true,
}

-- Returns true if this param should be skipped.
local function should_skip(param)
    if param.id == nil then return true end
    if param.setValue == nil then return true end
    if SKIP_TYPES[param.type] then return true end
    if type(param.default) ~= "number" then return true end

    local pmin = type(param.min) == "function" and param.min() or param.min
    local pmax = type(param.max) == "function" and param.max() or param.max

    -- No min/max: compound param (patches, SNA instrument selector, …)
    if pmin == nil or pmax == nil then return true end

    -- Dynamic MFX slot: toI7Value returns 0 for non-zero input when slot
    -- has no data (MFX type not yet set or slot index out of range).
    if param.toI7Value ~= nil then
        local ok, i7v = pcall(param.toI7Value, pmax)
        if not ok or (i7v == 0 and pmax ~= 0) then return true end
    end

    return false
end

-- Collect all leaf params from a section (flat + grouped).
local function collect_params(section)
    local result = {}
    if section.params then
        for _, p in ipairs(section.params) do table.insert(result, p) end
    end
    if section.grp then
        for _, g in ipairs(section.grp) do
            if g.params then
                for _, p in ipairs(g.params) do table.insert(result, p) end
            end
        end
    end
    return result
end

-- Test all testable params in a section: set default, receive back, assert.
local function test_section(section_key, section)
    local any = false
    for _, param in ipairs(collect_params(section)) do
        if should_skip(param) then
            skipped = skipped + 1
        else
            any = true
            SetParam(param.id, param.default)
            local got = request_param(param.id)
            assert_eq(got, param.default,
                section_key .. " / " .. param.id)
        end
    end
    return any
end

-- ── Section classification ────────────────────────────────────────────────────

-- Returns group name and part number (or nil) for a section key.
local function classify(key)
    local partStr, rest = string.match(key, "^(Part %d+) (.+)$")
    if not partStr then
        -- No part prefix → global sections
        if key == "System"                       then return "System",    nil end
        if key == "Motional Surround"            then return "StudioSet", nil end
        if key:find("Studio Set")               then return "StudioSet", nil end
        if key:find("Expansion")                then return "StudioSet", nil end
        return nil, nil   -- presets, parts meta, tab-only sections
    end

    local partNr = tonumber(string.match(partStr, "%d+"))

    -- Tone type from the section name suffix
    if rest:find("SN%-S")  then return "SNS",  partNr end
    if rest:find("SN%-D")  then return "SND",  partNr end
    if rest:find("PCM%-S") then return "PCMS", partNr end
    if rest:find("PCM%-D") then return "PCMD", partNr end
    if rest:find("SNA")    then return "SNA",  partNr end

    -- "Part XX MFX"  →  SNA's MFX (no other tone marker present)
    -- "Part XX Common" / "Part XX EQ" / … → studio-set part params
    if rest == "MFX" then return "SNA", partNr end
    return "Parts", partNr
end

-- ── Build ordered group list ──────────────────────────────────────────────────

-- Tone-type groups that need a precondition: index into NEFP_TYPE_DUMMY options
local TONE_TYPE_IDX = { SNA=1, SNS=2, SND=3, PCMS=4, PCMD=5 }

-- Collect sections per group for the test part (+ global sections).
local by_group = {}   -- group_name → list of {key, section}

for key, section in pairs(Main) do
    local group, partNr = classify(key)
    if group == nil then goto continue_classify end
    if partNr ~= nil and partNr ~= Config.test_part then goto continue_classify end

    if by_group[group] == nil then by_group[group] = {} end
    table.insert(by_group[group], {key=key, section=section})
    ::continue_classify::
end

-- Fixed group order so we set the tone type as rarely as possible.
local GROUP_ORDER = {"SNA", "SNS", "SND", "PCMS", "PCMD", "Parts", "System", "StudioSet"}

for _, group_name in ipairs(GROUP_ORDER) do
    if not Config.groups[group_name] then goto continue_order end
    local sections = by_group[group_name]
    if sections == nil or #sections == 0 then goto continue_order end

    local coro = coroutine.create(function()
        log(string.format("\n══ Group: %s ══", group_name))

        -- Set tone type precondition if needed.
        local tone_idx = TONE_TYPE_IDX[group_name]
        if tone_idx ~= nil then
            log(string.format("  → setting tone type %s for Part %d",
                group_name, Config.test_part))
            set_tone_type(tone_idx)
        end

        local section_count = 0
        for _, entry in ipairs(sections) do
            if test_section(entry.key, entry.section) then
                section_count = section_count + 1
            end
        end
        log(string.format("  sections tested: %d", section_count))
    end)

    table.insert(groups, {name=group_name, coro=coro})
    ::continue_order::
end

-- ── Frame driver (called by C++ every render frame) ──────────────────────────

function OnFrame()
    if done then return end

    if current == nil then
        if #groups == 0 then
            log(string.format(
                "\n── Device tests complete ──\n"..
                "   Passed:  %d\n"..
                "   Failed:  %d\n"..
                "   Skipped: %d\n",
                passed, failed, skipped))
            done = true
            return
        end
        local g = table.remove(groups, 1)
        log(string.format("\nRUN group: %s", g.name))
        current = g.coro
        coroutine.resume(current)
    elseif coroutine.status(current) == "suspended" then
        coroutine.resume(current)
    elseif coroutine.status(current) == "dead" then
        current = nil
    end
end
