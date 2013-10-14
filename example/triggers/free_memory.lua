--
-- free_memory.lua
--

return {
    dependencies = { "memory" },
    onData = function(state, value, timestamp)
        return value.MemFree, timestamp
    end
}
