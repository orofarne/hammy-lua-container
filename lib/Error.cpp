#include "Error.hpp"

namespace hammy {

Error::Error()
{
}

Error::Error(const char *message)
    : message_(message)
{
}

Error::Error(std::string const &message)
    : message_(message)
{

}

Error::Error(std::exception const &err) {
    message_ = err.what();
}

Error::~Error() throw() {

}

}
