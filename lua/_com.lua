HideParam = "__HIDDEN__"
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

GetWrapper = function (val)
    return function ()
        return val
    end
end

function MapArray(tbl, func)
    local new_tbl = {}
    for i, v in ipairs(tbl) do
        new_tbl[i] = func(v, i)
    end
    return new_tbl
end

function MapDict(tbl, func)
    local new_tbl = {}
    for i, v in pairs(tbl) do
        new_tbl[i] = func(v, i)
    end
    return new_tbl
end

function Concat(t1, t2)
    local result = {}
    for i = 1, #t1 do
        result[#result + 1] = t1[i]
    end
    for i = 1, #t2 do
        result[#result + 1] = t2[i]
    end
    return result
end