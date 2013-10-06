#include "Worker.hpp"

#include <string>
#include <sstream>
#include <stdexcept>

#include "lua/lua_global.hpp"

namespace hammy {

Worker::Worker(Context &c)
    : c_(c)
{

}

Worker::~Worker() throw() {

}

void *
Worker::operator()(void *in_buf, size_t in_size, size_t *out_buf) {
    // Unpack request
    lua_getglobal(c_.L_, "cmsgpack");
    lua_getfield(c_.L_, -1, "unpack");
    lua_pushlstring(c_.L_, reinterpret_cast<char *>(in_buf), in_size);
    int rc = lua_pcall(c_.L_, 1, 1, 0);
    if(rc) {
        std::string msg = "lua_pcall error: ";
        msg += lua_tolstring(c_.L_, -1, nullptr);
        lua_pop(c_.L_, 2); // errormessage, cmsgpack
        throw std::runtime_error(msg);
    }
    lua_setglobal(c_.L_, "__request");
    lua_pop(c_.L_, 1); // cmsgpack

    // Call function...
    lua_getglobal(c_.L_, HAMMY_PROCESS_REQUEST_FUNC);
    rc = lua_pcall(c_.L_, 1, 1, 0);
    if(rc) {
        std::string msg = "lua_pcall error: ";
        msg += lua_tolstring(c_.L_, -1, nullptr);
        throw std::runtime_error(msg);
    }

    // Pack response
    lua_getglobal(c_.L_, "cmsgpack");
    lua_getfield(c_.L_, -1, "pack");
    lua_getglobal(c_.L_, "__response");
    rc = lua_pcall(c_.L_, 1, 1, 0);
    if(rc) {
        std::string msg = "lua_pcall error: ";
        msg += lua_tolstring(c_.L_, -1, nullptr);
        lua_pop(c_.L_, 2); // errormessage, cmsgpack
        throw std::runtime_error(msg);
    }
    const char *resp = lua_tolstring(c_.L_, -1, out_buf);
    lua_pop(c_.L_, 1); // cmsgpack

    return reinterpret_cast<void *>(const_cast<char *>(resp));
}

}
