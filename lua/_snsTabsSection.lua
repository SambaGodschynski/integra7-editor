function CreateSnsTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " SN-S "

        local function tabsForPartials(group)
            local tabs = {}
            for i = 1, 3, 1 do
                table.insert(tabs, {
                    label = "Partial " .. i,
                    keys  = {prefix .. group .. " Partial " .. i}
                })
            end
            return tabs
        end

        local function hidePartials(group)
            for i = 1, 3, 1 do
                local s = main[prefix .. group .. " Partial " .. i]
                if s then s.hideFromPalette = true end
            end
        end

        -- OSC
        hidePartials("OSC")
        local k = prefix .. "OSC"
        main[k] = {name = k, tabs = tabsForPartials("OSC")}

        -- Pitch
        hidePartials("Pitch")
        k = prefix .. "Pitch"
        main[k] = {name = k, tabs = tabsForPartials("Pitch")}

        -- Filter
        hidePartials("Filter")
        k = prefix .. "Filter"
        main[k] = {name = k, tabs = tabsForPartials("Filter")}

        -- Amp
        hidePartials("Amp")
        k = prefix .. "Amp"
        main[k] = {name = k, tabs = tabsForPartials("Amp")}

        -- LFO
        hidePartials("LFO")
        k = prefix .. "LFO"
        main[k] = {name = k, tabs = tabsForPartials("LFO")}
    end
end
