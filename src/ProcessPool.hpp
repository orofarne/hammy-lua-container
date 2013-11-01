#pragma once

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
        ProcessPool(boost::asio::io_service &io_service,
                SubProcess::WorkerFunc w,
                unsigned int size, unsigned int max_count);
        ~ProcessPool() throw();

        void process(Buffer buf, Callback callback);

    private:
        struct Task {
            Buffer buf;
            Callback callback;
        };

        class CallbackProxy {
            public:
                CallbackProxy(ProcessPool *pp, Callback callback);
                ~CallbackProxy() throw();
                void operator()(Buffer buf, Error e);

            private:
                ProcessPool *pp_;
                Callback callback_;
        };

        friend class CallbackProxy;

    private:
        unsigned int pool_size_;
        unsigned int max_count_;
        std::vector<std::shared_ptr<SubProcess>> procs_;
        std::vector<unsigned int> counts_;

        boost::asio::io_service &io_service_;
        SubProcess::WorkerFunc w_;

        std::queue<Task> task_queue_;
};

}
