#include "Application.hpp"

#include <stdexcept>

namespace hammy {

Application::Application(boost::property_tree::ptree &config)
    : config_(config)
{

}

Application::~Application() throw() {

}

void
Application::loadPlugin(std::string const &file) {
    PluginFactory f = loader_.load(file);

    std::shared_ptr<Plugin> p{ f(config_) };
    if(!p)
        throw std::runtime_error("Plugin is null");
}

Bus &
Application::bus() {
    if(!bus_)
        throw std::runtime_error("Bus plugin is not configured");
    return *bus_;
}

StateKeeper &
Application::stateKeeper() {
    if(!state_keeper_)
        throw std::runtime_error("StateKeeper plugin is not configured");
    return *state_keeper_;
}


}
