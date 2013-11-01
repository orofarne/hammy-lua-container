#include "ProcessPool.hpp"

#include <algorithm>
#include <functional>
#include <sstream>

#include "Worker.hpp"
#include "msglen.h"

#define BLOCK_SIZE 1024

namespace hammy {

ProcessPool::ProcessPool(boost::asio::io_service &io_service,
        SubProcess::WorkerFunc w,
        unsigned int size, unsigned int max_count)
    : pool_size_(size)
    , max_count_(max_count)
    , procs_(pool_size_)
    , counts_(pool_size_)
    , io_service_(io_service)
    , w_(w)
{
    std::fill(counts_.begin(), counts_.end(), 0);
}

ProcessPool::~ProcessPool() throw () {

}

void
ProcessPool::process(Buffer buf, Callback callback) {
    if(task_queue_.empty()) {
        // Look for free worker
        for(int i = 0; i < pool_size_; ++i) {
            std::shared_ptr<SubProcess> &sp = procs_[i];

            if(sp && !sp->isFree())
                continue;

            if(!sp || counts_[i] > max_count_) {
                sp.reset(new SubProcess{io_service_, w_});
                sp->fork();
            }

            ++counts_[i];

            sp->process(buf, callback);

            return;
        }
    }

    // All workers is busy
    Task t;
    t.buf = buf;
    t.callback = CallbackProxy{this, callback};
    task_queue_.push(t);
}

ProcessPool::CallbackProxy::CallbackProxy(ProcessPool *pp,
        ProcessPool::Callback callback)
    : pp_(pp)
    , callback_(callback)
{
}

ProcessPool::CallbackProxy::~CallbackProxy() throw() {
}

void
ProcessPool::CallbackProxy::operator()(Buffer buf, Error e) {
    pp_->io_service_.post(std::bind(callback_, buf, e));

    if(!pp_->task_queue_.empty()) {
        Task t = pp_->task_queue_.front();
        pp_->task_queue_.pop();

        // Look for free worker
        for(int i = 0; i < pp_->pool_size_; ++i) {
            std::shared_ptr<SubProcess> &sp = pp_->procs_[i];

            if(sp && !sp->isFree())
                continue;

            if(!sp || pp_->counts_[i] > pp_->max_count_) {
                sp.reset(new SubProcess{pp_->io_service_, pp_->w_});
                sp->fork();
            }

            ++pp_->counts_[i];

            sp->process(buf, CallbackProxy{pp_, callback_});
            return;
        }

        std::ostringstream msg;
        msg << "On " << __FILE__ << ":" << __LINE__;
        throw std::logic_error(msg.str());
    }
}

}
