#pragma once

#include "Value.hpp"

namespace hammy {

struct Request {
    std::string func; // text
    std::string state; // binary
    std::string metric; // text
    Value value;
    time_t timestamp;
};

}
