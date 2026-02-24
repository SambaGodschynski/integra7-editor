function CreatePcmsTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " PCM-S "

        local function tabsForPartials(group)
            local tabs = {}
            for i = 1, 4, 1 do
                table.insert(tabs, {
                    label = "Partial " .. i,
                    keys  = {prefix .. group .. " Partial " .. i}
                })
            end
            return tabs
        end

        -- Hide all per-partial single-view sections that are covered by a tab wrapper.
        local function hidePartials(group)
            for i = 1, 4, 1 do
                local s = main[prefix .. group .. " Partial " .. i]
                if s then s.hideFromPalette = true end
            end
        end
        local function hideKey(key)
            local s = main[key]
            if s then s.hideFromPalette = true end
        end

        -- Wave
        hidePartials("Wave")
        local k = prefix .. "Wave"
        main[k] = {name = k, tabs = tabsForPartials("Wave")}

        -- PMT  (common section rendered above partial tabs)
        hideKey(prefix .. "PMT Common")
        hidePartials("PMT")
        k = prefix .. "PMT"
        main[k] = {name = k, tabCommonKey = prefix .. "PMT Common",
                   tabs = tabsForPartials("PMT")}

        -- Pitch  ("Pitch" key is taken by the common-pitch section → use "Pitch All")
        hideKey(prefix .. "Pitch")
        hidePartials("Pitch")
        k = prefix .. "Pitch All"
        main[k] = {name = k, tabCommonKey = prefix .. "Pitch",
                   tabs = tabsForPartials("Pitch")}

        -- Pitch Env
        hidePartials("Pitch Env")
        k = prefix .. "Pitch Env"
        main[k] = {name = k, tabs = tabsForPartials("Pitch Env")}

        -- TVF
        hidePartials("TVF")
        k = prefix .. "TVF"
        main[k] = {name = k, tabs = tabsForPartials("TVF")}

        -- TVF Env
        hidePartials("TVF Env")
        k = prefix .. "TVF Env"
        main[k] = {name = k, tabs = tabsForPartials("TVF Env")}

        -- TVA
        hidePartials("TVA")
        k = prefix .. "TVA"
        main[k] = {name = k, tabs = tabsForPartials("TVA")}

        -- TVA Env
        hidePartials("TVA Env")
        k = prefix .. "TVA Env"
        main[k] = {name = k, tabs = tabsForPartials("TVA Env")}

        -- LFO1 / LFO2 / Step LFO  (each as its own tab group)
        hidePartials("LFO1")
        k = prefix .. "LFO1"
        main[k] = {name = k, tabs = tabsForPartials("LFO1")}

        hidePartials("LFO2")
        k = prefix .. "LFO2"
        main[k] = {name = k, tabs = tabsForPartials("LFO2")}

        hidePartials("Step LFO")
        k = prefix .. "Step LFO"
        main[k] = {name = k, tabs = tabsForPartials("Step LFO")}

        -- CTRL
        hidePartials("CTRL")
        k = prefix .. "CTRL"
        main[k] = {name = k, tabs = tabsForPartials("CTRL")}

        -- Output
        hidePartials("Output")
        k = prefix .. "Output"
        main[k] = {name = k, tabs = tabsForPartials("Output")}

        -- MTRX CTRL  (tabs per control 1..4, not per partial)
        for i = 1, 4, 1 do hideKey(prefix .. "MTRX CTRL " .. i) end
        k = prefix .. "MTRX CTRL"
        local mtrxTabs = {}
        for i = 1, 4, 1 do
            table.insert(mtrxTabs, {
                label = "CTRL " .. i,
                keys  = {prefix .. "MTRX CTRL " .. i}
            })
        end
        main[k] = {name = k, tabs = mtrxTabs}
    end
end
