function CreatePcmsTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " PCM-S "

        local function hideKey(key)
            local s = main[key]
            if s then s.hideFromPalette = true end
        end

        local function hidePartials(group)
            for i = 1, 4, 1 do
                local s = main[prefix .. group .. " Partial " .. i]
                if s then s.hideFromPalette = true end
            end
        end

        -- Hide per-partial sections (embedded as accordions in Tone tabs)
        hidePartials("Wave")
        hidePartials("PMT")
        hidePartials("Pitch")
        hidePartials("Pitch Env")
        hidePartials("TVF")
        hidePartials("TVF Env")
        hidePartials("TVA")
        hidePartials("TVA Env")
        hidePartials("LFO1")
        hidePartials("LFO2")
        hidePartials("Step LFO")
        hidePartials("CTRL")
        hidePartials("Output")
        for i = 1, 4, 1 do hideKey(prefix .. "MTRX CTRL " .. i) end

        -- Hide old tab-wrapper sections
        hideKey(prefix .. "Wave")
        hideKey(prefix .. "PMT")
        hideKey(prefix .. "Pitch All")
        hideKey(prefix .. "Pitch Env")
        hideKey(prefix .. "TVF")
        hideKey(prefix .. "TVF Env")
        hideKey(prefix .. "TVA")
        hideKey(prefix .. "TVA Env")
        hideKey(prefix .. "LFO1")
        hideKey(prefix .. "LFO2")
        hideKey(prefix .. "Step LFO")
        hideKey(prefix .. "CTRL")
        hideKey(prefix .. "MTRX CTRL")
        hideKey(prefix .. "Output")

        -- Hide common sub-sections now merged into Common
        hideKey(prefix .. "Pitch")       -- Pitch Bend common
        hideKey(prefix .. "PMT Common")  -- PMT structure/booster common

        -- Tone tab group: 4 partial tabs, each with 14 accordion sections
        local toneKey = prefix .. "Tone"
        local tabs = {}
        for i = 1, 4, 1 do
            local pi = tostring(i)
            table.insert(tabs, {
                label = "Partial " .. i,
                keys = {
                    {key = prefix .. "Wave Partial "      .. pi, accordion = "Wave"},
                    {key = prefix .. "PMT Partial "       .. pi, accordion = "PMT"},
                    {key = prefix .. "Pitch Partial "     .. pi, accordion = "Pitch"},
                    {key = prefix .. "Pitch Env Partial " .. pi, accordion = "Pitch Env"},
                    {key = prefix .. "TVF Partial "       .. pi, accordion = "TVF"},
                    {key = prefix .. "TVF Env Partial "   .. pi, accordion = "TVF Env"},
                    {key = prefix .. "TVA Partial "       .. pi, accordion = "TVA"},
                    {key = prefix .. "TVA Env Partial "   .. pi, accordion = "TVA Env"},
                    {key = prefix .. "LFO1 Partial "      .. pi, accordion = "LFO1"},
                    {key = prefix .. "LFO2 Partial "      .. pi, accordion = "LFO2"},
                    {key = prefix .. "Step LFO Partial "  .. pi, accordion = "Step LFO"},
                    {key = prefix .. "CTRL Partial "      .. pi, accordion = "CTRL"},
                    {key = prefix .. "MTRX CTRL "         .. pi, accordion = "MTRX"},
                    {key = prefix .. "Output Partial "    .. pi, accordion = "Output"},
                }
            })
        end
        main[toneKey] = {name = toneKey, tabs = tabs}
    end
end
