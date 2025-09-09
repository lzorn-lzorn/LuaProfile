-- mod1.lua
return function()
    local s = 0
    for i = 1, 10000 do
        for j = 1, 10 do
            s = s + i * j
        end
        if i % 100 == 0 then
            s = s - math.floor(s / (i + 1))
        end
    end
    return s
end
