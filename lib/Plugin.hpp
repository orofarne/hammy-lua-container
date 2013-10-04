#pragma once

#include <boost/property_tree/ptree.hpp>

namespace hammy {

class Plugin {
    public:
        Plugin() {}
        virtual ~Plugin() throw() {};
};

using PluginFactory = Plugin *(*)(boost::property_tree::ptree &config);

}
