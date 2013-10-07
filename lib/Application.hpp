#pragma once

#include "Plugin.hpp"
#include "PluginLoader.hpp"

#include "Bus.hpp"
#include "StateKeeper.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/asio.hpp>

#include <string>
#include <map>
#include <memory>

namespace hammy {

class Application {
    public:
        Application(boost::property_tree::ptree &config);
        virtual ~Application() throw();

        inline boost::asio::io_service &ioService() { return io_service_; }
        inline boost::property_tree::ptree &config() { return config_; }

        Bus &bus();
        StateKeeper &stateKeeper();

    private:
        void loadPlugin(std::string const &file);

    private:
        // Boost.Asio staff
        boost::asio::io_service io_service_;

        // Config
        boost::property_tree::ptree &config_;

        // Plugins
        PluginLoader loader_;
        std::map<std::string, std::shared_ptr<Plugin>> plugins_;

        std::shared_ptr<Bus> bus_;
        std::shared_ptr<StateKeeper> state_keeper_;
};

}
