-- mod2.lua
return function()
    local s = 1.0
    for i = 1, 10000 do
        for j = 1, 5 do
            s = s * (1.00001 + j * 0.000001)
        end
        if i % 500 == 0 then
            s = s / (i + 1)
        end
    end
    return s
end
