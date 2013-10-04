#include "PluginLoader.hpp"

#include <dlfcn.h>

#include <stdexcept>

namespace hammy {

PluginLoader::PluginLoader() {

}

PluginLoader::~PluginLoader() throw() {

}

PluginFactory
PluginLoader::load(std::string const &file) {
    std::shared_ptr<void> p{
            ::dlopen(file.c_str(), RTLD_NOW | RTLD_GLOBAL),
            &PluginLoader::xdlclose
        };

    if(p.get() == nullptr)
        throw std::runtime_error(dlerror());

    PluginFactory factory = reinterpret_cast<PluginFactory>(dlsym(p.get(), "plugin_factory"));
    if(factory == nullptr)
        throw std::runtime_error(dlerror());

    handles_.push_back(p);

    return factory;
}

void
PluginLoader::xdlclose(void *ptr) {
    if(ptr != nullptr)
        dlclose(ptr);
}

}
