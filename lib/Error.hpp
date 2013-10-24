#pragma once

#include <string>
#include <exception>

namespace hammy {

class Error {
    public:
        Error();
        Error(const char *message);
        Error(std::string const &message);
        Error(std::exception const &err);

        virtual ~Error() throw();

        inline bool isError() { return message_.empty(); }
        inline std::string const &message() { return message_; }

    private:
        std::string message_;
};

}
