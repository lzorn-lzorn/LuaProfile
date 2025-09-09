-- main.lua
local mod1 = require("mod1")
local mod2 = require("mod2")
local mod3 = require("mod3")

local result = 0
for i = 1, 10 do
	result = result + mod1() * i
	result = result + mod2() / (i + 1)
	result = result + mod3() - i
end


return result
