#pragma once

#include "Client.hpp"
#include "ProcessPool.hpp"

#include <boost/asio.hpp>

#include <string>
#include <map>
#include <memory>

namespace hammy {

class Server {
    public:
        explicit Server(std::string sock_path, ProcessPool &pp);
        virtual ~Server() throw();

        inline boost::asio::io_service &ioService() { return io_service_; }

    private:
        void acceptCb(boost::system::error_code ec);
        void dataCb(Client *c, Client::Buffer b, Error e);

    private:
        // Boost.Asio staff
        boost::asio::io_service io_service_;
        boost::asio::local::stream_protocol::endpoint ep_;
        boost::asio::local::stream_protocol::acceptor acceptor_;
        std::shared_ptr<boost::asio::local::stream_protocol::socket> socket_;

        // Clients
        std::vector<std::shared_ptr<Client>> clients_;

        // Lua staff
        ProcessPool &pp_;
};

}
