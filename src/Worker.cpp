#include "Worker.hpp"

#include <string>
#include <sstream>
#include <stdexcept>

#include <string.h>

#include "lua/lua_global.hpp"

namespace hammy {

Worker::Worker(Context &c)
    : c_(c)
{

}

Worker::~Worker() throw() {

}

void *
Worker::operator()(void *in_buf, size_t in_size, size_t *out_size) {
    // Unpack request
    lua_getglobal(c_.L_, "cmsgpack");
    lua_getfield(c_.L_, -1, "unpack");
    lua_remove(c_.L_, -2); // cmsgpack
    lua_pushlstring(c_.L_, reinterpret_cast<char *>(in_buf), in_size);
    int rc = lua_pcall(c_.L_, 1, 1, 0);
    if(rc) {
        std::string msg = "lua_pcall [cmsgpack.unpack] error: ";
        msg += lua_tolstring(c_.L_, -1, nullptr);
        lua_pop(c_.L_, 1); // errormessage
        throw std::runtime_error(msg);
    }
    lua_setglobal(c_.L_, "__request");

    // Call function...
    lua_getglobal(c_.L_, HAMMY_PROCESS_REQUEST_FUNC);
    if(!lua_isfunction(c_.L_, -1)) {
        std::string msg = HAMMY_PROCESS_REQUEST_FUNC;
        msg += " is not a function";
        throw std::runtime_error(msg);
    }
    rc = lua_pcall(c_.L_, 0, 0, 0);
    if(rc) {
        std::string msg = "lua_pcall [";
        msg += HAMMY_PROCESS_REQUEST_FUNC;
        msg += "] error: ";
        msg += lua_tolstring(c_.L_, -1, nullptr);
        throw std::runtime_error(msg);
    }

    // Pack response
    lua_getglobal(c_.L_, "cmsgpack");
    lua_getfield(c_.L_, -1, "pack");
    lua_remove(c_.L_, -2); // cmsgpack
    lua_getglobal(c_.L_, "__response");
    rc = lua_pcall(c_.L_, 1, 1, 0);
    if(rc) {
        std::string msg = "lua_pcall [cmsgpack.pack] error: ";
        msg += lua_tolstring(c_.L_, -1, nullptr);
        lua_pop(c_.L_, 1); // errormessage
        throw std::runtime_error(msg);
    }
    const char *resp = lua_tolstring(c_.L_, -1, out_size);

    void *out_buf = ::malloc(*out_size);
    ::memcpy(out_buf, resp, *out_size);

    return out_buf;
}

}
