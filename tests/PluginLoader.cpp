#include <gtest/gtest.h>

#include "PluginLoader.hpp"

#include <memory>
#include <boost/asio.hpp>

#include "test_config.h"
#include "TestPlugin.hpp"

using namespace hammy;

TEST(PluginLoader, Empty) {
    PluginLoader pl;
}

TEST(PluginLoader, TestPlugin) {
    PluginLoader pl;

    PluginFactory f = pl.load(TEST_PLUGIN_FILE);
    ASSERT_NE(nullptr, f);

    boost::asio::io_service io;
    boost::property_tree::ptree config;
    std::shared_ptr<Plugin> p{ f(io, config) };
    ASSERT_NE(nullptr, p.get());

    TestPlugin *plugin = dynamic_cast<TestPlugin *>(p.get());
    EXPECT_EQ(8, plugin->plus(3, 5));
}
