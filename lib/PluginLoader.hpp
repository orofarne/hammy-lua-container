#pragma once

#include "Plugin.hpp"

#include <memory>
#include <vector>
#include <string>

namespace hammy {

class PluginLoader {
    public:
        PluginLoader();
        ~PluginLoader() throw();

        PluginFactory load(std::string const &file);

    private:
        std::vector<std::shared_ptr<void>> handles_;

    private:
        static void xdlclose(void *ptr);
};

}
