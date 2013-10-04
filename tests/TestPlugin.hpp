#pragma once

#include "Plugin.hpp"

namespace hammy {

class TestPlugin : public Plugin {
    public:
        TestPlugin() {}
        virtual ~TestPlugin() throw() {}

        virtual int plus(int a, int b) = 0;
};

}
