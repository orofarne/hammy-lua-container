#include "../TestPlugin.hpp"

#include <boost/property_tree/ptree.hpp>

namespace hammy {

class TestPluginImpl : public TestPlugin {
    public:
        TestPluginImpl() {}
        virtual ~TestPluginImpl() throw() {}

        virtual int plus(int a, int b) { return a + b; }
};

}

extern "C" hammy::Plugin *plugin_factory(boost::asio::io_service &io, boost::property_tree::ptree &config) {
    using namespace hammy;

    return new TestPluginImpl{};
}

