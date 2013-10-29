#pragma once

#include <exception>
#include <memory>

namespace hammy {

using Error = std::shared_ptr<std::exception>;

}
