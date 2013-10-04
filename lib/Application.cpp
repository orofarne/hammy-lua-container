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

}
