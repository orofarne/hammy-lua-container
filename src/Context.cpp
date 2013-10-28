#include "Context.hpp"

#include "lua_cmsgpack.h"

#include <stdexcept>
#include <sstream>

#include <string.h>

#define HAMMY_ASSERT_EQ(x, y, msg, L) { \
    if((x) != (y)) { \
        std::ostringstream sstr; \
        sstr << msg << ": " \
            << #x << " is " << (x) << " (should be " << (y) << ")"; \
        if(L != nullptr) { \
            sstr << " Message: " << lua_tostring(L, -1); \
            lua_pop(L, 1); \
        } \
        throw std::runtime_error(sstr.str()); \
    } \
}

#include "lua/lua_global.hpp"

namespace hammy {

Context::Context()
    : L_(nullptr)
{
    L_ = lua_open();

    // stdlib
    luaL_openlibs(L_);

    // cmsgpack
    int rc = luaopen_cmsgpack(L_);
    HAMMY_ASSERT_EQ(rc, 1, "luaopen_cmsgpack", L_);

    // luajit
    rc = luaJIT_setmode(L_, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON);
    HAMMY_ASSERT_EQ(rc, 1, "luaJIT_setmode", nullptr);

    // hammy_lua_global
    load(lua_global_code, strlen(lua_global_code));
}

Context::~Context() throw() {
    if(L_ != nullptr) {
        lua_close(L_);
    }
}

void
Context::load(const char *code, size_t len) {
    int rc = luaL_loadbuffer(L_, code, len, "hammy");
    HAMMY_ASSERT_EQ(rc, 0, "luaL_loadbuffer", L_);

    rc = lua_pcall(L_, 0, 0, 0);
    if(rc) {
        std::string msg = "lua_pcall error: ";
        msg += lua_tolstring(L_, -1, nullptr);
        lua_pop(L_, 1);
        throw std::runtime_error(msg);
    }
}

void
Context::loadModule(const char *name, const char *code, size_t len) {
    int rc = luaL_loadbuffer(L_, code, len, "hammy");
    HAMMY_ASSERT_EQ(rc, 0, "luaL_loadbuffer", L_);

    rc = lua_pcall(L_, 0, 1, 0);
    if(rc) {
        std::string msg = "lua_pcall error: ";
        msg += lua_tolstring(L_, -1, nullptr);
        lua_pop(L_, 1);
        throw std::runtime_error(msg);
    }

    lua_setglobal(L_, name);
}

std::shared_ptr<LuaValue>
Context::operator[](const char *name) {
    lua_getglobal(L_, name);
    return std::make_shared<LuaValue>(L_);
}

void
Context::setString(const char *name, const char *str) {
    lua_pushstring(L_, str);
    lua_setglobal(L_, name);
}

void
Context::setString(const char *name, const char *str, size_t len) {
    lua_pushlstring(L_, str, len);
    lua_setglobal(L_, name);
}

}
