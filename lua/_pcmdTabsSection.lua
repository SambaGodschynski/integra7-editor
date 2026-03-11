function CreatePcmdTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " PCM-D "

        -- WMT tabs (4 wave slots) with note selector above the tab bar
        local kWmtNote = prefix .. "WMT Note"
        local wmtTabs = {}
        for wmt = 1, 4, 1 do
            local wKey = prefix .. "WMT " .. wmt
            table.insert(wmtTabs, {label = "WMT " .. wmt, keys = {wKey}})
            local s = main[wKey]
            if s then s.hideFromPalette = true end
        end
        local sNote = main[kWmtNote]
        if sNote then sNote.hideFromPalette = true end
        local kWmt = prefix .. "WMT"
        main[kWmt] = {name = kWmt, tabs = wmtTabs, tabCommonKey = kWmtNote}

        -- CompEq tabs (6 channels)
        local compEqTabs = {}
        for ch = 1, 6, 1 do
            local chKey = prefix .. "CompEq " .. ch
            table.insert(compEqTabs, {label = "Comp+EQ " .. ch, keys = {chKey}})
            local s = main[chKey]
            if s then s.hideFromPalette = true end
        end
        local kEq = prefix .. "CompEq"
        main[kEq] = {name = kEq, tabs = compEqTabs}
    end
end
