--
-- memory.lua - meminfo parser
--

return {
    period = 10,
    onTimer = function(state)
        return 1
        --[[
        local res = {}
        for line in io.lines('/proc/meminfo') do
            local it = string.gmatch(line, "%S+")
            local name = it()
            local val = it()
            name = string.sub(name, 1, -2)
            res[name] = val
        end
        return res
        ]]
    end
}
