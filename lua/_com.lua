function DeepCopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for k, v in pairs(orig) do
            copy[DeepCopy(k)] = DeepCopy(v)
        end
        -- Metatable (optional)
        setmetatable(copy, getmetatable(orig))
    else
        copy = orig
    end
    return copy
end