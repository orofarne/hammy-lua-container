#pragma once

#include <string>
#include <memory>
#include <exception>

namespace hammy {

using Error = std::shared_ptr<std::exception>;
using Buffer = std::shared_ptr<std::string>;

}
