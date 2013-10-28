#pragma once

#include "Context.hpp"
#include "SubProcess.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include <tuple>
#include <queue>
#include <memory>
#include <functional>

#include <boost/asio/io_service.hpp>

namespace hammy {

class ProcessPool {
    public:
        typedef std::function<void(std::shared_ptr<Response>)> RequestCallback;

    public:
        ProcessPool(boost::asio::io_service &io_service, Context &cx, unsigned int size, unsigned int max_count);
        ~ProcessPool() throw();

        void process(std::shared_ptr<Request> r, RequestCallback callback);

    private:
        struct Connection {
            size_t len;
            size_t offset;
            char *p;

            Connection();
            ~Connection() throw();
        };

    private:
        unsigned int pool_size_;
        unsigned int max_count_;
        std::vector<std::shared_ptr<SubProcess>> procs_;
        std::vector<unsigned int> counts_;
        std::vector<bool> in_use_;
        std::queue<std::tuple<std::shared_ptr<Request>, RequestCallback>>  task_queue_;

        boost::asio::io_service &io_service_;
        Context &c_;

    private:
        void process_proc(std::shared_ptr<SubProcess> proc, std::shared_ptr<Request> r, RequestCallback callback);
        std::shared_ptr<SubProcess> getProc();
        void writer(std::shared_ptr<std::string> buffer,
                std::shared_ptr<SubProcess> proc, RequestCallback callback,
                boost::system::error_code ec, size_t n);
        void reader(std::shared_ptr<Connection> conn,
                std::shared_ptr<SubProcess> proc, RequestCallback callback,
                boost::system::error_code ec, size_t n);
};

}
