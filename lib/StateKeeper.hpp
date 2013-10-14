#pragma once

#include "Plugin.hpp"

namespace hammy {

class StateKeeper : public Plugin {
    public:
        StateKeeper() {}
        virtual ~StateKeeper() throw() {}

        virtual void set(std::string host, std::string metric, std::string buffer) = 0;
        virtual void get(std::string host, std::string metric, std::string *buffer) = 0;
};

}
