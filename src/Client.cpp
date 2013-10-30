#include "Client.hpp"

#include <functional>

namespace hammy {

Client::Client(std::shared_ptr<boost::asio::local::stream_protocol::socket> s,
                    Callback callback)
    : socket_(s)
    , callback_(callback)
{
    namespace ph = std::placeholders;

    reader_.reset(
            new Reader<boost::asio::local::stream_protocol::socket>(
                socket_->get_io_service(), *socket_,
                std::bind(&Client::newData, this, ph::_1)
                )
            );
}


Client::~Client() throw() {
    socket_->close();
}

void
Client::newData(Error e) {
    if(e) {
        socket_->get_io_service().dispatch(
                std::bind(callback_, this, nullptr, e)
            );
        return;
    }

    size_t len;
    char *ptr = reader_->get(&len);
    Buffer b{new std::string{ptr, len}};
    reader_->pop();

    socket_->get_io_service().dispatch(
                std::bind(callback_, this, b, nullptr)
            );
}


void
Client::write(Buffer b, ErrorCallback callback) {
    // TODO
}

}
