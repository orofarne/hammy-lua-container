--
-- worker.lua
--

function hammy_process_request()
    __response = {}

    local module = _G[__request.module]
    local state = __request.state or {}
    local mt = {
        __index = function(t, key)
            local ret = module[key]
            if ret ~= nil then
                return ret
            else
                return rawget(t, key)
            end
        end
    }

    setmetatable(state, mt)

    local rc, res, ts = pcall(module[__request.func], state, __request.value, __request.timestamp)

    setmetatable(state, nil)

    if rc then
        __response['state'] = state
        __response['value'] = res
        __response['timestamp'] = ts
    else
        __response['error'] = res
    end
end
