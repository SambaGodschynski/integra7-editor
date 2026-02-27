function CreateSndTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " SN-D "

        -- CompEq tabs (6 channels)
        local compEqTabs = {}
        for ch = 1, 6, 1 do
            local chKey = prefix .. "CompEq " .. ch
            table.insert(compEqTabs, {label = "Comp+EQ " .. ch, keys = {chKey}})
            local s = main[chKey]
            if s then s.hideFromPalette = true end
        end
        local k = prefix .. "CompEq"
        main[k] = {name = k, tabs = compEqTabs}
    end
end
