#pragma once

#include "Reader.hpp"

#include <memory>
#include <functional>
#include <boost/asio.hpp>

namespace hammy {

class Client {
    public:
        using Buffer = std::shared_ptr<std::string>;
        using Callback = std::function<void(Client *, Buffer, Error)>;
        using ErrorCallback = std::function<void(Error)>;

    public:
        Client(const Client&) = delete;
        Client(Client &&) = delete;
        Client& operator=(const Client&) = delete;

        explicit Client(std::shared_ptr<boost::asio::local::stream_protocol::socket> s,
                            Callback callback);

        ~Client() throw();

        void write(Buffer b, ErrorCallback callback);

    private:
        void newData(Error e);

    private:
        std::shared_ptr<boost::asio::local::stream_protocol::socket> socket_;
        std::shared_ptr<Reader<boost::asio::local::stream_protocol::socket>> reader_;
        Callback callback_;
};

}
