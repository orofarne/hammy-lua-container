#pragma once

#include "Value.hpp"

#include <string>

namespace hammy {

struct Response {
    std::string state; // binary
    Value value; // binary
    time_t timestamp;
    std::string error; // text
};

}
