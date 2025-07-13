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

local parameter_partId_placeholder = "xxx"

function CreateId(parameter_node_id_template, part_id)
    return string.gsub(parameter_node_id_template, parameter_partId_placeholder, part_id)
end