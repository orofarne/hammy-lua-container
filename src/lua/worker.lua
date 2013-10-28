--
-- worker.lua
--

function hammy_process_request()
    __response = {}

    local code = __request.code
    local f, err = loadstring(code)
    if not f then
        __response['error'] = err
        return
    end
    local module = f()

    local state = __request.state or {}
    local mt = {
        __index = function(t, key)
            local ret = module[key]
            if ret ~= nil then
                return ret
            end

            for _, v in pairs(module.extends or {}) do
                local smodule = _G[v]
                ret = smodule[key]
                if ret ~= nil then
                    return ret
                end
            end

            return rawget(t, key)
        end
    }

    setmetatable(state, mt)

    local rc, res, ts = pcall(state[__request.func], state, __request.value, __request.timestamp)

    setmetatable(state, nil)

    if rc then
        __response['state'] = state
        __response['value'] = res
        __response['timestamp'] = ts
    else
        __response['error'] = res
    end
end
