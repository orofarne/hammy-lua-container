#pragma once

#include <string>

namespace hammy {

struct Request {
    uint64_t id;
    std::string code; // text
    std::string func; // text
    std::string data; // binary
};

}
