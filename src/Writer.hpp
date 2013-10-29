#pragma once

#include <boost/asio.hpp>

#include <queue>
#include <memory>
#include <cassert>

namespace hammy {

template<typename T>
class Writer {
    public:
        using Callback = std::function<void(Error)>;

    public:
        Writer(boost::asio::io_service &io, T &sock, Callback err_callback)
            : io_(io)
            , sock_(sock)
            , err_callback_(err_callback)
            , current_offset_(0)
        {
        }

        ~Writer() throw() {}

        void write(char *buf, size_t len);

    private:
        struct Buffer {
            char *ptr;
            size_t len;

            Buffer() : ptr(nullptr), len(0) {}
            ~Buffer() throw() { if(ptr != nullptr) ::free(ptr); }
        };

    private:
        boost::asio::io_service &io_;
        T &sock_;

        Callback err_callback_;

        std::queue<std::shared_ptr<Buffer>> write_queue_;
        std::shared_ptr<Buffer> current_buffer_;
        size_t current_offset_;

    private:
        void writeSome();
        void writeFunc(boost::system::error_code ec, size_t n);
};

template<typename T>
void
Writer<T>::write(char *buf, size_t len) {
    std::shared_ptr<Buffer> b{new Buffer};
    assert(b);
    b->ptr = buf;
    b->len = len;
    write_queue_.push(b);

    writeSome();
}

template<typename T>
void
Writer<T>::writeSome() {
    namespace ph = std::placeholders;

    if(current_buffer_ || write_queue_.empty()) {
        return;
    }

    current_buffer_ = write_queue_.front();
    current_offset_ = 0;
    write_queue_.pop();
    assert(current_buffer_);

    sock_.async_write_some(
            boost::asio::buffer(
                current_buffer_->ptr + current_offset_,
                current_buffer_->len - current_offset_
                ),
            std::bind(
                &Writer<T>::writeFunc,
                this, ph::_1, ph::_2
                )
            );
}

template<typename T>
void
Writer<T>::writeFunc(boost::system::error_code ec, size_t n) {
    if(ec) {
        Error e;
        std::ostringstream msg;
        msg << "async_write error: " << ec.message();
        e.reset(new std::runtime_error(msg.str()));
        io_.dispatch(std::bind(err_callback_, e));
        return;
    }

    current_offset_ += n;
    if(current_offset_ < current_buffer_->len) {
        writeSome();
        return;
    }

    current_buffer_.reset();

    writeSome();
}

}
