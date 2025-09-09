-- mod3.lua
return function()
    local s = 0
    for i = 1, 10000 do
        for j = 1, 3 do
            s = s + math.sin(i * j) + math.cos(i + j) - math.tan(j)
        end
        if i % 200 == 0 then
            s = s * math.abs(math.sin(i))
        end
    end
    return s
end
