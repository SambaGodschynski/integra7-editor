function CreateSnsTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " SN-S "

        -- Hide individual partial sections from palette (embedded in Tone tabs)
        local function hidePartials(group)
            for i = 1, 3, 1 do
                local s = main[prefix .. group .. " Partial " .. i]
                if s then s.hideFromPalette = true end
            end
        end

        hidePartials("OSC")
        hidePartials("Pitch")
        hidePartials("Filter")
        hidePartials("Amp")
        hidePartials("LFO")

        -- Hide Misc from palette (merged into Common)
        local miscKey = prefix .. "Misc"
        if main[miscKey] then main[miscKey].hideFromPalette = true end

        -- Hide per-partial Ctrl sections from palette (shown inside Tone tabs)
        for i = 1, 3, 1 do
            local ctrlKey = prefix .. "Partial " .. i .. " Ctrl"
            if main[ctrlKey] then main[ctrlKey].hideFromPalette = true end
        end

        -- Tone tab group: Switch/Select header + OSC/Pitch/Filter/Amp/LFO accordions per Partial
        local toneKey = prefix .. "Tone"
        local tabs = {}
        for i = 1, 3, 1 do
            local pi = tostring(i)
            table.insert(tabs, {
                label = "Partial " .. i,
                keys = {
                    {key = prefix .. "Partial " .. pi .. " Ctrl"},
                    {key = prefix .. "OSC Partial "    .. pi, accordion = "OSC"},
                    {key = prefix .. "Pitch Partial "  .. pi, accordion = "Pitch"},
                    {key = prefix .. "Filter Partial " .. pi, accordion = "Filter"},
                    {key = prefix .. "Amp Partial "    .. pi, accordion = "Amp"},
                    {key = prefix .. "LFO Partial "    .. pi, accordion = "LFO"},
                }
            })
        end
        main[toneKey] = {name = toneKey, tabs = tabs}
    end
end
