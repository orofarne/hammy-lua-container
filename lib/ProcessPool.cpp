#include "ProcessPool.hpp"

#include <algorithm>
#include <functional>

#include "Worker.hpp"
#include "Encode.hpp"
#include "msglen.h"

#define BLOCK_SIZE 1024

namespace hammy {

ProcessPool::ProcessPool(boost::asio::io_service &io_service, Context &cx, unsigned int size, unsigned int max_count)
    : pool_size_(size)
    , max_count_(max_count)
    , procs_(pool_size_)
    , counts_(pool_size_)
    , in_use_(pool_size_)
    , io_service_(io_service)
    , c_(cx)
{
    std::fill(counts_.begin(), counts_.end(), 0);
    std::fill(in_use_.begin(), in_use_.end(), false);
}

ProcessPool::~ProcessPool() throw () {

}

ProcessPool::Connection::Connection()
    : len(0)
    , offset(0)
    , p(nullptr)
{
}

ProcessPool::Connection::~Connection() throw() {
    if(p != nullptr)
        ::free(p);
}

void
ProcessPool::process(std::shared_ptr<Request> r, RequestCallback callback) {
    auto proc = getProc();

    if(proc) {
        process_proc(proc, r, callback);
    }
    else {
        task_queue_.push(
            std::tuple<std::shared_ptr<Request>, RequestCallback>(r, callback)
        );
    }
}

std::shared_ptr<SubProcess>
ProcessPool::getProc() {
    for(int i = 0; i < pool_size_; ++i) {
        if(in_use_[i])
            continue;

        std::shared_ptr<SubProcess> proc = procs_[i];
        if(!proc || counts_[i] >= max_count_) {
            proc.reset(new SubProcess{io_service_, Worker{c_}});
            proc->fork();
            counts_[i] = 0;
        }

        in_use_[i] = true;
        return proc;
    }
    return nullptr;
}

void
ProcessPool::process_proc(std::shared_ptr<SubProcess> proc,
        std::shared_ptr<Request> r, RequestCallback callback)
{
    std::shared_ptr<std::string> buffer{new std::string{encodeRequest(*r)}};

    boost::asio::async_write(proc->downD(), boost::asio::buffer(*buffer),
        std::bind(&ProcessPool::writer, this, buffer, proc, callback,
            std::placeholders::_1, std::placeholders::_2)
        );
}

void
ProcessPool::writer(std::shared_ptr<std::string> buffer,
                std::shared_ptr<SubProcess> proc, RequestCallback callback,
                boost::system::error_code ec, size_t n)
{
    if(ec) {
        std::ostringstream msg;
        msg << "async_write error: " << ec.message();
        throw std::runtime_error(msg.str());
    }

    if(n < buffer->size()) {
        std::ostringstream msg;
        msg << "writer: " << n << " bytes from " << buffer->size();
        throw std::runtime_error(msg.str());
    }

    std::shared_ptr<Connection> conn{new Connection};
    conn->len = BLOCK_SIZE;
    conn->p = reinterpret_cast<char *>(::malloc(conn->len));
    if(conn->p == nullptr)
        throw std::runtime_error("no memory");

    proc->upD().async_read_some(boost::asio::buffer(conn->p, conn->len),
        std::bind(&ProcessPool::reader, this, conn, proc, callback,
            std::placeholders::_1, std::placeholders::_2)
    );
}

void
ProcessPool::reader(std::shared_ptr<Connection> conn,
                std::shared_ptr<SubProcess> proc, RequestCallback callback,
                boost::system::error_code ec, size_t n)
{
    if(ec) {
        std::ostringstream msg;
        msg << "async_read error: " << ec.message();
        throw std::runtime_error(msg.str());
    }

    char *error = nullptr;
    size_t msg_len = ::msgpackclen_buf_read(conn->p, n, &error);
    if(error) {
        std::ostringstream msg;
        msg << "Error [msgpackclen_buf_read]: " << error;
        ::free(error);
        throw std::runtime_error{msg.str()};
    }

    conn->offset += n;

    if(msg_len == 0) {
        if(n == conn->len - conn->offset) {
            conn->len += BLOCK_SIZE;
            conn->p = reinterpret_cast<char *>(::realloc(conn->p, conn->len));
            if(conn->p == nullptr)
                throw std::runtime_error("no memory");

            proc->upD().async_read_some(
                boost::asio::buffer(conn->p + conn->offset, conn->len - conn->offset),
                std::bind(&ProcessPool::reader, this, conn, proc, callback,
                    std::placeholders::_1, std::placeholders::_2)
            );
        }
    }
    else {
        auto it = std::find(procs_.cbegin(), procs_.cend(), proc);
        int k = it - procs_.cbegin();

        counts_[k] += 1;

        auto r = decodeResponse(std::string{conn->p, conn->len});

        callback(r);

        if(!task_queue_.empty()) {
            auto task = task_queue_.front();
            io_service_.dispatch(std::bind(&ProcessPool::process_proc,
                        this, proc, std::get<0>(task), std::get<1>(task)));
            task_queue_.pop();
        }
        else {
            in_use_[k] = false;
        }
    }
}

}
