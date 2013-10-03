#pragma once

#include "Request.hpp"
#include "Response.hpp"

#include <memory>

namespace hammy {

std::string encodeRequest(const Request &r);
std::shared_ptr<Response> decodeResponse(std::string const &buf);

}
