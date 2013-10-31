#pragma once

#include "Context.hpp"
#include "SubProcess.hpp"

#include "types.hpp"

#include <tuple>
#include <queue>
#include <memory>
#include <functional>

#include <boost/asio/io_service.hpp>

namespace hammy {

class ProcessPool {
    public:
        using Callback = std::function<void(Buffer, Error)>;

    public:
        ProcessPool(boost::asio::io_service &io_service, Context &cx, unsigned int size, unsigned int max_count);
        ~ProcessPool() throw();

        void process(Buffer buf, Callback callback);

    private:
        unsigned int pool_size_;
        unsigned int max_count_;
        std::vector<std::shared_ptr<SubProcess>> procs_;

        boost::asio::io_service &io_service_;
        Context &c_;
};

}
