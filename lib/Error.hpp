#pragma once

#include <string>
#include <exception>

namespace hammy {

class Error {
    public:
        Error(const char *message);
        Error(std::string const &message);
        Error(std::exception const &err);

        virtual ~Error() throw();

        inline std::string const &message() { return message_; }

    private:
        std::string message_;
};

}
