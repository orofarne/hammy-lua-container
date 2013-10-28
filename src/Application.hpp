#pragma once

#include <boost/asio.hpp>

#include <string>
#include <map>
#include <memory>

namespace hammy {

class Application {
    public:
        Application();
        virtual ~Application() throw();

        inline boost::asio::io_service &ioService() { return io_service_; }

    private:
        // Boost.Asio staff
        boost::asio::io_service io_service_;
};

}
