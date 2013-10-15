#pragma once

#include "Value.hpp"

#include <vector>

namespace hammy {

struct Request {
    std::string code; // text
    std::string func; // text

    std::string host; // text
    std::string metric; // text
    std::string state; // binary
    Value value;
    time_t timestamp;
};

}
