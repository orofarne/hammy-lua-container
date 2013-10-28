#pragma once

#include <string>

namespace hammy {

struct Response {
    uint64_t id;
    std::string data; // binary
    std::string error; // text
};

}
