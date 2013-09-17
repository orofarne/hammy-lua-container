#pragma once

#include "lua.hpp"

namespace lutop {

class Context {
    public:
        Context();
        ~Context() throw();

        void load(const char *code, size_t len);
        void loadModule(const char *name, const char *code, size_t len);

        void setString(const char *name, const char *str);
        void setString(const char *name, const char *str, size_t len);
        const char *getString(const char *name, size_t *len = NULL);

        void mixTatables(const char *name1, const char *name2);

    private:
        lua_State *L_;

    private:
        static int lIndexPass_(lua_State *L);
};

}
