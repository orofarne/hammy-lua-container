#pragma once

#include "Value.hpp"

namespace hammy {

struct Request {
    std::string host; // text
    std::string metric; // text
    std::string func; // text
    std::string state; // binary
    Value value;
    time_t timestamp;
};

}
