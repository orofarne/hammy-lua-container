#pragma once

#include "luatools.hpp"

namespace hammy {

class Context {
    public:
        Context();
        ~Context() throw();

        void load(const char *code, size_t len);
        void loadModule(const char *name, const char *code, size_t len);

        std::shared_ptr<LuaValue> operator[](const char *name);

        void setString(const char *name, const char *str);
        void setString(const char *name, const char *str, size_t len);

    private:
        lua_State *L_;

    private:
        friend class Worker;
};

}
