#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/property_tree/ptree.hpp>

namespace hammy {

class Plugin {
    public:
        Plugin() {}
        virtual ~Plugin() throw() {};
};

using PluginFactory = Plugin *(*)(boost::asio::io_service &io, boost::property_tree::ptree &config);

}
