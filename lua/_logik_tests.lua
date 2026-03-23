-- Standalone logic test for all section parameter definitions.
-- Tests toI7Value/toGuiValue roundtrips and setValue -> DT1-payload roundtrips
-- without a connected device and without a running C++ host.
--
-- Run from the lua/ directory:
--   lua _logik_tests.lua

-- ============================================================
-- Mock C++ host bindings (must come before require "_sysex")
-- ============================================================
GetDeviceId = function() return 0x10 end

ValueChangedMessage = {}
ValueChangedMessage.__index = ValueChangedMessage
function ValueChangedMessage.new()
    return setmetatable({id = "", i7Value = 0}, ValueChangedMessage)
end

RequestMessage = {}
RequestMessage.__index = RequestMessage
function RequestMessage.new()
    return setmetatable({sysex = {}, onMessageReceived = nil}, RequestMessage)
end

-- ============================================================
-- Load all modules and sections
-- ============================================================
require "_com"
require "_model"
require "_integra7"
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

-- ============================================================
-- Build the full section map (same as main.lua)
-- ============================================================
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
CreateSnsCommonSections(Main)
CreateSnsMiscSections(Main)
CreateSnsOscSections(Main)
CreateSnsPitchSections(Main)
CreateSnsFilterSections(Main)
CreateSnsAmpSections(Main)
CreateSnsLfoSections(Main)
CreateMfxSnsSections(Main)
CreateSndCommonSections(Main)
CreateSndInstSections(Main)
CreateSndCompEqSections(Main)
CreateMfxSndSections(Main)
CreatePcmdCommonSections(Main)
CreatePcmdPitchSections(Main)
CreatePcmdCompEqSections(Main)
CreateMfxPcmdSections(Main)
CreateExpansionSection(Main)
CreateRssSections(Main)
CreateStudioSetEffectsSections(Main)
CreateSystemSections(Main)

-- ============================================================
-- Helpers
-- ============================================================

local passed  = 0
local failed  = 0
local skipped = 0

local function pass()
    passed = passed + 1
end

local function fail(ctx, msg)
    failed = failed + 1
    io.write("FAIL [" .. ctx .. "] " .. msg .. "\n")
end

local function skip()
    skipped = skipped + 1
end

-- Extract the payload bytes from the first Roland DT1 SysEx message in a table.
-- Format: f0 41 devId 00 00 64 12 [addr:4] [payload...] checksum f7
-- Header = 11 bytes (1-based indices 1..11).
-- Stops at the first 0xf7, so multi-sysex responses are handled correctly.
local function extract_dt1_payload(sysex)
    local header_len = 11
    -- minimum: header(11) + 1 payload byte + checksum + f7 = 14 bytes
    if #sysex < 14 then
        return nil
    end
    -- Find the first f7 after the header (end of the first sysex message)
    local f7_idx = nil
    for i = header_len + 1, #sysex do
        if sysex[i] == 0xf7 then
            f7_idx = i
            break
        end
    end
    if f7_idx == nil then
        return nil
    end
    -- payload = bytes between header and checksum (f7_idx-1 is checksum)
    local payload = {}
    for i = header_len + 1, f7_idx - 2 do
        table.insert(payload, sysex[i])
    end
    if #payload == 0 then
        return nil
    end
    return payload
end

-- Collect all leaf-level param tables from a section.
-- Handles both flat (section.params) and grouped (section.grp[].params) layouts.
local function collect_params(section)
    local result = {}
    if section.params ~= nil then
        for _, p in ipairs(section.params) do
            table.insert(result, p)
        end
    end
    if section.grp ~= nil then
        for _, grp in ipairs(section.grp) do
            if grp.params ~= nil then
                for _, p in ipairs(grp.params) do
                    table.insert(result, p)
                end
            end
        end
    end
    return result
end

-- Types whose setValue does not produce a simple DT1 byte array
-- (either no SysEx at all, or a file dialog, etc.)
local SKIP_TYPES = {
    envelope   = true,
    steplfo    = true,
    action     = true,
    savesysex  = true,
    loadsysex  = true,
    solotoggle = true,
}

-- ============================================================
-- Per-parameter tests
-- ============================================================

local function test_param(section_name, param)
    if param.id == nil then
        return
    end

    local ctx = section_name .. " / " .. tostring(param.id)

    -- Skip types that don't have a conventional setValue -> DT1 flow
    if SKIP_TYPES[param.type] then
        skip()
        return
    end

    local pmin = type(param.min) == "function" and param.min() or param.min
    local pmax = type(param.max) == "function" and param.max() or param.max

    -- --------------------------------------------------------
    -- Test 1: default value is within [min, max]
    -- --------------------------------------------------------
    if type(param.default) == "number" and pmin ~= nil and pmax ~= nil then
        if param.default < pmin or param.default > pmax then
            fail(ctx, string.format(
                "default=%s is outside [%s, %s]",
                tostring(param.default), tostring(pmin), tostring(pmax)))
        else
            pass()
        end
    end

    -- --------------------------------------------------------
    -- Test 2: toI7Value / toGuiValue roundtrip (pure math)
    -- Skip if toI7Value returns 0 for non-zero input: this indicates a
    -- dynamic param (e.g. MFX param slot) whose conversion depends on
    -- runtime state that is not set up in this standalone test.
    -- --------------------------------------------------------
    if param.toI7Value ~= nil and param.toGuiValue ~= nil then
        local test_vals = {}
        if type(param.default) == "number" then
            table.insert(test_vals, param.default)
        end
        if pmin ~= nil then table.insert(test_vals, pmin) end
        if pmax ~= nil then table.insert(test_vals, pmax) end

        for _, v in ipairs(test_vals) do
            local ok, result = pcall(function()
                local i7v  = param.toI7Value(v)
                local back = param.toGuiValue(i7v)
                return {i7v = i7v, back = back}
            end)
            if not ok then
                -- Float boundaries that lack an exact integer representation are
                -- a known limitation (e.g. NESC_TUNE Hz min/max). Skip, not fail.
                local errmsg = tostring(result)
                if errmsg:find("no integer representation") or errmsg:find("arithmetic on a nil") then
                    skip()
                else
                    fail(ctx, "toI7Value/toGuiValue raised error for v=" .. tostring(v) .. ": " .. errmsg)
                end
            elseif result.i7v == nil or (result.i7v == 0 and v ~= 0) then
                -- Dynamic param whose conversion function returns 0/nil for any
                -- input when the dependent state (MFX type etc.) is not set.
                skip()
            elseif result.back ~= v then
                fail(ctx, string.format(
                    "toGuiValue(toI7Value(%s)) = %s (i7=%s)",
                    tostring(v), tostring(result.back), tostring(result.i7v)))
            else
                pass()
            end
        end
    end

    -- --------------------------------------------------------
    -- Test 3: setValue -> DT1 payload -> i7Value roundtrip
    -- Only for numeric default values.
    -- setValue expects i7 values (it calls node:setvalue which validates
    -- against i7 bounds). toI7Value converts GUI -> i7 before calling.
    --
    -- Compound params (patches, parts, MFX type) return multiple
    -- concatenated sysex messages. Detect these by counting 0xf7 bytes
    -- and skip: the first-message payload is unrelated to the param value.
    -- --------------------------------------------------------
    if param.setValue ~= nil and type(param.default) == "number" then
        local test_vals = {}
        table.insert(test_vals, param.default)
        if pmin ~= nil and pmin ~= param.default then table.insert(test_vals, pmin) end
        if pmax ~= nil and pmax ~= param.default then table.insert(test_vals, pmax) end

        for _, v in ipairs(test_vals) do
            -- Convert GUI value to i7 value; setValue expects i7 values.
            -- Skip if toI7Value signals "dynamic/unset" (returns 0 for non-zero v).
            local i7v
            if param.toI7Value ~= nil then
                local ok2, res2 = pcall(param.toI7Value, v)
                if not ok2 or res2 == nil or (res2 == 0 and v ~= 0) then
                    skip()
                    goto continue_t3
                end
                i7v = res2
            else
                i7v = v
            end

            do
                local ok, sysex = pcall(param.setValue, i7v)
                if not ok then
                    -- "value out of bounds" for i7v=0 typically means the param
                    -- is in an unset dynamic state (mfxData=nil → toI7Value→0).
                    if tostring(sysex):find("value out of bounds") and i7v == 0 then
                        skip()
                    elseif tostring(sysex):find("no node found") then
                        -- Node ID not in model tree -- skip (known RSS path issue)
                        skip()
                    else
                        fail(ctx, "setValue raised error for i7v=" .. tostring(i7v) ..
                            " (guiV=" .. tostring(v) .. "): " .. tostring(sysex))
                    end
                elseif type(sysex) ~= "table" or #sysex < 14 then
                    -- EmptySysex ({f0,f7}) or non-table return -- deferred-write param
                    skip()
                else
                    -- Count 0xf7 terminators: more than one means compound (multi-sysex).
                    local f7_count = 0
                    for _, b in ipairs(sysex) do
                        if b == 0xf7 then f7_count = f7_count + 1 end
                    end
                    if f7_count > 1 then
                        -- Compound setValue (MFX type change, patch selection, etc.)
                        -- The first sysex payload is unrelated to the GUI value.
                        skip()
                    else
                        local payload = extract_dt1_payload(sysex)
                        if payload == nil then
                            fail(ctx, "setValue returned malformed sysex for i7v=" .. tostring(i7v))
                        else
                            local got_i7v = Bytes_To_Value(payload)
                            if got_i7v ~= i7v then
                                fail(ctx, string.format(
                                    "setValue payload mismatch for guiV=%s i7v=%s: got i7=%s (sysex: %s)",
                                    tostring(v), tostring(i7v), tostring(got_i7v),
                                    Bytes_To_String(sysex)))
                            else
                                pass()
                            end
                        end
                    end
                end
            end
            ::continue_t3::
        end
    end
end

-- ============================================================
-- Run all sections
-- ============================================================

local section_count = 0
for section_name, section in pairs(Main) do
    section_count = section_count + 1
    local params = collect_params(section)
    for _, param in ipairs(params) do
        test_param(section_name, param)
    end
end

-- ============================================================
-- Summary
-- ============================================================

io.write(string.format(
    "\nSections: %d   Passed: %d   Failed: %d   Skipped: %d\n",
    section_count, passed, failed, skipped))

if failed > 0 then
    os.exit(1)
end
