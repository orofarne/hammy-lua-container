#include "SubProcess.hpp"

#include "msglen.h"

#include <string>

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BLOCK_SIZE 1024

namespace hammy {

SubProcess::subprocess_error::~subprocess_error() {
}

SubProcess::subprocess_error::subprocess_error(const char *comment)
    : runtime_error(std::string("Subprocess error [")
            + comment + " ] : " + strerror(errno)) // Not thread-safe!!!
{
}

SubProcess::SubProcess(boost::asio::io_service &io_service, WorkerFunc f)
    : pid_(0)
    , worker_f_(f)
    , io_service_(io_service)
    , up_d_(io_service_)
    , down_d_(io_service_)
{
    if(pipe(pipefd_down_.data()) < 0)
        throw subprocess_error("pipe");
    if(fcntl(pipefd_down_[1], F_SETFL, O_NONBLOCK) < 0)
        throw subprocess_error("fcntl");

    if(pipe(pipefd_up_.data()) < 0)
        throw subprocess_error("pipe");
    if(fcntl(pipefd_up_[0], F_SETFL, O_NONBLOCK) < 0)
        throw subprocess_error("fcntl");
}

SubProcess::~SubProcess() throw() {
    for(int fd : pipefd_down_) {
        close(fd);
    }
    for(int fd : pipefd_up_) {
        close(fd);
    }

    if(pid_ > 0) {
        if(kill(pid_, SIGKILL) < 0 && errno != ESRCH) {
            assert(0);
            // log it?...
        }

        // Not good place for waitpid :-(
    }
}

void
SubProcess::fork() {
    namespace ph = std::placeholders;

    pid_t cpid = ::fork();

    if(cpid < 0)
        throw subprocess_error("fork");

    io_service_.notify_fork(boost::asio::io_service::fork_prepare);
    if(cpid == 0) {
        // Child process
        io_service_.notify_fork(boost::asio::io_service::fork_child);

        close(pipefd_down_[1]);
        close(pipefd_up_[0]);

        up_d_.assign(pipefd_up_[1]);
        down_d_.assign(pipefd_down_[0]);

        reader_.reset(
            new Reader<boost::asio::posix::stream_descriptor>(
                io_service_, down_d_,
                std::bind(&SubProcess::newDataChild, this, ph::_1)
                )
            );

        writer_.reset(
            new Writer<boost::asio::posix::stream_descriptor>(
                io_service_, up_d_,
                std::bind(&SubProcess::newDataChild, this, ph::_1)
                )
            );
    } else {
        // Parent process
        io_service_.notify_fork(boost::asio::io_service::fork_parent);

        pid_ = cpid;

        close(pipefd_down_[0]);
        close(pipefd_up_[1]);

        up_d_.assign(pipefd_up_[0]);
        down_d_.assign(pipefd_down_[1]);

        reader_.reset(
            new Reader<boost::asio::posix::stream_descriptor>(
                io_service_, up_d_,
                std::bind(&SubProcess::newDataParent, this, ph::_1)
                )
            );

        writer_.reset(
            new Writer<boost::asio::posix::stream_descriptor>(
                io_service_, down_d_,
                std::bind(&SubProcess::newDataParent, this, ph::_1)
                )
            );
    }
}

void
SubProcess::newDataChild(Error e) {
    if(e) {
        throw std::runtime_error(e->what());
    }

    size_t len = 0;
    char *ptr = reader_->get(&len);

    Buffer res = worker_f_(ptr, len);

    reader_->pop();

    writer_->write(res);
}

void
SubProcess::newDataParent(Error e) {
    if(callback_) {
        if(e) {
            io_service_.dispatch(std::bind(callback_, nullptr, e));
        }
        else {
            size_t len;
            char *ptr = reader_->get(&len);
            Buffer b{ new std::string{ptr, len} };
            reader_->pop();

            io_service_.post(std::bind(callback_, b, e));
            callback_ = Callback{};
        }
    }
    else {
        if(!e)
            e.reset(new std::runtime_error{"Unexpected data"});

        error_ = e;
    }
}

void
SubProcess::process(Buffer b, Callback callback) {
    callback_ = callback;
    writer_->write(b);
}

}
