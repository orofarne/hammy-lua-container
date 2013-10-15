#pragma once

#include "Plugin.hpp"

#include <string>
#include <vector>

namespace hammy {

class CodeLoader : public Plugin {
    public:
        CodeLoader() {}
        virtual ~CodeLoader() throw() {}

        virtual std::vector<std::string> loadTrigger(std::string const &host, std::string const &metric) = 0;
};

}
