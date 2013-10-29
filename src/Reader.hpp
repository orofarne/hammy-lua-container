#pragma once

#include "Error.hpp"

#include "msglen.h"

#include <stdexcept>
#include <functional>
#include <sstream>
#include <cassert>
#include <queue>
#include <malloc.h>

#include <iostream> // DEBUG

#include <boost/asio.hpp>

#define BLOCK_SIZE 1024

namespace hammy {

template<typename T>
class Reader {
    public:
        using Callback = std::function<void(Error)>;

    public:
        Reader(boost::asio::io_service &io, T &sock, Callback callback)
            : io_(io)
            , sock_(sock)
            , len_(0)
            , offset_(0)
            , shift_(0)
            , callback_(callback)
        {
            cap_ = BLOCK_SIZE;
            ptr_ = reinterpret_cast<char *>(::malloc(cap_));

            readSome();
        }

        ~Reader() throw() {
            if(ptr_ != nullptr)
                ::free(ptr_);
        }

        char *get(size_t *len);
        void pop();

    private:
        void readSome();
        void readFunc(boost::system::error_code ec, size_t n);

    private:
        boost::asio::io_service &io_;
        T &sock_;

        size_t len_;
        size_t cap_;
        size_t offset_;
        size_t shift_;
        std::queue<size_t> msg_lens_;
        char *ptr_;

        Callback callback_;
};

template<typename T>
void
Reader<T>::readSome() {
    namespace ph = std::placeholders;

    sock_.async_read_some(
            boost::asio::buffer(ptr_ + len_, cap_ - len_),
            std::bind(&Reader<T>::readFunc, this, ph::_1, ph::_2)
            );
}

template<typename T>
char *
Reader<T>::get(size_t *len) {
    if(msg_lens_.empty())
        return nullptr;

    if(len != nullptr)
        *len = msg_lens_.front();
    return ptr_ + shift_;
}

template<typename T>
void
Reader<T>::pop() {
    size_t msg_len = msg_lens_.front();

    shift_ += msg_len;
    msg_lens_.pop();
}

template<typename T>
void
Reader<T>::readFunc(boost::system::error_code ec, size_t n) {
    if(ec) {
        Error e;
        std::ostringstream msg;
        msg << "async_read error: " << ec.message();
        e.reset(new std::runtime_error(msg.str()));
        io_.dispatch(std::bind(callback_, e));
        return;
    }

    len_ += n;

    if(shift_ > 0) {
        ::memmove(ptr_, ptr_ + shift_, len_ - shift_);
        len_ -= shift_;
        offset_ -= shift_;
        shift_ = 0;
    }

    while(true) {
        char *err = nullptr;
        size_t msg_len = ::msgpackclen_buf_read(ptr_ + offset_, len_ - offset_, &err);
        if(err) {
            Error e;
            std::ostringstream msg;
            msg << "Error [msgpackclen_buf_read]: " << err;
            ::free(err);
            e.reset(new std::runtime_error{msg.str()});
            io_.dispatch(std::bind(callback_, e));
            return;
        }

        if(msg_len == 0) {
            if(len_ == cap_) {
                cap_ += BLOCK_SIZE;
                ptr_ = reinterpret_cast<char *>(::realloc(ptr_, cap_));
                if(ptr_ == nullptr) {
                    Error e;
                    e.reset(new std::runtime_error{"no memory"});
                    io_.dispatch(std::bind(callback_, e));
                    return;
                }
            }
            break;
        }
        else {
            assert(len_ >= msg_len);

            msg_lens_.push(msg_len);
            offset_ += msg_len;

            Error e;
            io_.post(std::bind(callback_, e));
        }
    }

    readSome();
}

}

#undef BLOCK_SIZE
