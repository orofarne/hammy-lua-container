#include "Application.hpp"

#include <stdexcept>

namespace hammy {

Application::Application(boost::property_tree::ptree &config)
    : config_(config)
{
    boost::property_tree::ptree h_cfg = config.get_child("hammy");

    std::string plugin_path = h_cfg.get<std::string>("plugin_path");

    bus_ = loadPlugin(
        plugin_path + "/lib" + h_cfg.get<std::string>("bus_plugin") + ".so",
        h_cfg.get<std::string>("bus_plugin")
    );

    state_keeper_ = loadPlugin(
        plugin_path + "/lib" + h_cfg.get<std::string>("state_keeper_plugin")
                                                                        + ".so",
        h_cfg.get<std::string>("state_keeper_plugin")
    );
}

Application::~Application() throw() {

}

std::shared_ptr<Plugin>
Application::loadPlugin(std::string const &file, std::string const &section) {
    PluginFactory f = loader_.load(file);

    boost::property_tree::ptree cfg = config_.get_child(section);

    std::shared_ptr<Plugin> p{ f(cfg) };
    if(!p)
        throw std::runtime_error("Plugin is null");

    return p;
}

Bus &
Application::bus() {
    if(!bus_)
        throw std::runtime_error("Bus plugin is not configured");
    return dynamic_cast<Bus &>(*bus_);
}

StateKeeper &
Application::stateKeeper() {
    if(!state_keeper_)
        throw std::runtime_error("StateKeeper plugin is not configured");
    return dynamic_cast<StateKeeper &>(*state_keeper_);
}


}
