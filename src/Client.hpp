#pragma once

#include "Reader.hpp"
#include "Writer.hpp"

#include "types.hpp"

#include <boost/asio.hpp>

namespace hammy {

class Client {
    public:
        using Callback = std::function<void(Client *, Buffer, Error)>;

    public:
        Client(const Client&) = delete;
        Client(Client &&) = delete;
        Client& operator=(const Client&) = delete;

        explicit Client(std::shared_ptr<boost::asio::local::stream_protocol::socket> s,
                            Callback callback);

        ~Client() throw();

        void write(Buffer b);

    private:
        void newData(Error e);

    private:
        std::shared_ptr<boost::asio::local::stream_protocol::socket> socket_;
        std::shared_ptr<Reader<boost::asio::local::stream_protocol::socket>> reader_;
        std::shared_ptr<Writer<boost::asio::local::stream_protocol::socket>> writer_;
        Callback callback_;
};

}
