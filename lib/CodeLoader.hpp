#pragma once

#include "Plugin.hpp"

#include <string>
#include <vector>

namespace hammy {

class CodeLoader : public Plugin {
    public:
        CodeLoader() {}
        virtual ~CodeLoader() throw() {}

        virtual void loadTrigger(std::string const &host, std::string const &metric, std::vector<std::string> *res, PluginCallback callback) = 0;
};

}
