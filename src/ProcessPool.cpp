#include "ProcessPool.hpp"

#include <algorithm>
#include <functional>

#include "Worker.hpp"
#include "msglen.h"

#define BLOCK_SIZE 1024

namespace hammy {

ProcessPool::ProcessPool(boost::asio::io_service &io_service, Context &cx, unsigned int size, unsigned int max_count)
    : pool_size_(size)
    , max_count_(max_count)
    , procs_(pool_size_)
    , io_service_(io_service)
    , c_(cx)
{
}

ProcessPool::~ProcessPool() throw () {

}

void
ProcessPool::process(Buffer buf, Callback callback) {
    // TODO
}

}
