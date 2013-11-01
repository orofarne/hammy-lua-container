#pragma once

#include "Reader.hpp"
#include "Writer.hpp"

#include <stdexcept>
#include <array>
#include <functional>

#include <boost/asio.hpp>

namespace hammy {

class SubProcess {
    public:
        class subprocess_error : public std::runtime_error {
            public:
                virtual ~subprocess_error();
                explicit subprocess_error(const char *comment);
        };

        using WorkerFunc = std::function<Buffer(char *buf, size_t len)>;
        using Callback = std::function<void(Buffer buf, Error e)>;

    public:
        SubProcess(boost::asio::io_service &io_service, WorkerFunc f);
        ~SubProcess() throw();

        void fork();
        void process(Buffer b, Callback callback);
        inline bool isFree() { return !callback_; }
        inline Error error() { return error_; }

    private:
        std::array<int, 2> pipefd_down_;
        std::array<int, 2> pipefd_up_;
        int pid_;

        WorkerFunc worker_f_;
        Callback callback_;
        Error error_;

        // Boost::asio staff
        boost::asio::io_service &io_service_;
        boost::asio::posix::stream_descriptor up_d_;
        boost::asio::posix::stream_descriptor down_d_;

        std::shared_ptr<Reader<boost::asio::posix::stream_descriptor>> reader_;
        std::shared_ptr<Writer<boost::asio::posix::stream_descriptor>> writer_;

    private:
        void newDataChild(Error e);
        void newDataParent(Error e);
};

}
