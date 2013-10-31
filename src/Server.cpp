#include "Server.hpp"

#include <stdexcept>
#include <functional>
#include <algorithm>

namespace hammy {

Server::Server(std::string sock_path, ProcessPool &pp)
    : ep_(sock_path)
    , acceptor_(io_service_, ep_)
    , socket_(new boost::asio::local::stream_protocol::socket{io_service_})
    , pp_(pp)
{
    namespace ph = std::placeholders;

    acceptor_.async_accept(*socket_, std::bind(&Server::acceptCb, this, ph::_1));
}

Server::~Server() throw() {

}

void
Server::acceptCb(boost::system::error_code ec) {
    namespace ph = std::placeholders;

    if (!acceptor_.is_open())   {
        return;
    }

    if (!ec) {
        std::shared_ptr<Client> c{
            new Client{
                socket_,
                std::bind(&Server::dataCb, this, ph::_1, ph::_2, ph::_3)
            }
        };
        clients_.push_back(c);
    }
}

void
Server::dataCb(Client *c, Buffer b, Error e) {
    if(e) {
        std::remove_if(clients_.begin(), clients_.end(),
            [&](std::shared_ptr<Client> cl) { return cl.get() == c; }
        );
        return;
    }

    pp_.process(b, [=](Buffer rb, Error e) {
            if(e) {
                std::remove_if(clients_.begin(), clients_.end(),
                    [&](std::shared_ptr<Client> cl) { return cl.get() == c; }
                );
                return;
            }

            c->write(rb);
        });
}

}
