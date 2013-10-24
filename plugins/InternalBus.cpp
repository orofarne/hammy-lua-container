#include "Bus.hpp"

#include <boost/asio.hpp>

#include <iostream>
#include <functional>

namespace hammy {

class InternalBus : public Bus {
        boost::asio::io_service &io_;

        BusCallback callback_;

    public:
        InternalBus(boost::asio::io_service &io)
            : io_(io)
        {}
        virtual ~InternalBus() throw() {}

        virtual void push(std::string host, std::string metric, Value value, time_t timestamp) {
            std::cout << host << "\t" << metric << "\t"
                << value.as<std::string>() << std::endl;

            io_.dispatch(std::bind(callback_, host, metric, value, timestamp));
        }

        virtual void setCallback(BusCallback callback) {
            callback_ = callback;
        }
};

}

extern "C" hammy::Plugin *plugin_factory(boost::asio::io_service &io, boost::property_tree::ptree &config) {
    (void)config;

    return dynamic_cast<hammy::Plugin *>(new hammy::InternalBus{io});
}
