#pragma once

#include <memory>
#include <functional>
#include <malloc.h>

#include <boost/asio.hpp>

#define BLOCK_SIZE 1024

namespace hammy {

template<typename T>
class Reader {
    public:
        Reader(boost::asio::io_service &io, T &sock)
            : io_(io)
            , sock_(sock)
            , ptr_(nullptr, &Reader::xfree)
        {
            len_ = BLOCK_SIZE;
            ptr_.reset(reinterpret_cast<char *>(::malloc(len_)));
        }

        ~Reader() throw() {}

        static void xfree(char *ptr) {
            if(ptr)
                ::free(ptr);
        }

    private:
        boost::asio::io_service &io_;
        T &sock_;

        size_t len_;
        std::shared_ptr<char> ptr_;
};

}

#undef BLOCK_SIZE
