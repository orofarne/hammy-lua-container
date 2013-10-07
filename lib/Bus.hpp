#pragma once

#include "Value.hpp"
#include "Plugin.hpp"

#include <functional>

namespace hammy {

class Bus : public Plugin {
    public:
        using BusCallback = std::function<void(std::string metric, Value value, time_t timestamp)>;
    public:
        Bus() {}
        virtual ~Bus() throw() {}

        virtual void push(std::string metric, Value value, time_t timestamp) = 0;
        virtual void setCallback(BusCallback callback) = 0;
};

}
