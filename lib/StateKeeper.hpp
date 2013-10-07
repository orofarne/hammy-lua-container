#pragma once

#include "Plugin.hpp"

namespace hammy {

class StateKeeper : public Plugin {
    public:
        StateKeeper() {}
        virtual ~StateKeeper() throw() {}

        virtual void set(std::string metric, std::string buffer) = 0;
        virtual std::string get(std::string metric) = 0;
};

}
