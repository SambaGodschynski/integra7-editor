require "math"

local NoteNames = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"}

local function noteName(note)
    local octave = math.floor(note / 12) - 1
    local name = NoteNames[(note % 12) + 1]
    return name .. tostring(octave)
end

function CreateSndTabSections(main)
    for partNr = 1, 16, 1 do
        local pn     = string.format("%02d", partNr)
        local prefix = "Part " .. pn .. " SN-D "

        -- Drum Inst tabs (notes 27-88)
        local instTabs = {}
        for note = 27, 88, 1 do
            local nn   = noteName(note)
            local chKey = prefix .. "Inst " .. nn
            table.insert(instTabs, {label = nn, keys = {chKey}})
            local s = main[chKey]
            if s then s.hideFromPalette = true end
        end
        local k = prefix .. "Inst"
        main[k] = {name = k, tabs = instTabs}

        -- CompEq tabs (6 channels)
        local compEqTabs = {}
        for ch = 1, 6, 1 do
            local chKey = prefix .. "CompEq " .. ch
            table.insert(compEqTabs, {label = "Comp+EQ " .. ch, keys = {chKey}})
            local s = main[chKey]
            if s then s.hideFromPalette = true end
        end
        k = prefix .. "CompEq"
        main[k] = {name = k, tabs = compEqTabs}
    end
end
