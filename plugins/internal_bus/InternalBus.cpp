#include "Bus.hpp"

#include <iostream>

namespace hammy {

class InternalBus : public Bus {
        BusCallback callback_;

    public:
        InternalBus() {}
        virtual ~InternalBus() throw() {}

        virtual void push(std::string metric, Value value, time_t timestamp) {
            std::cout << metric << "\t" << value.as<std::string>() << std::endl;

            callback_(metric, value, timestamp);
        }

        virtual void setCallback(BusCallback callback) {
            callback_ = callback;
        }
};

}

extern "C" hammy::Plugin *plugin_factory(boost::property_tree::ptree &config) {
    (void)config;

    return dynamic_cast<hammy::Plugin *>(new hammy::InternalBus{});
}
