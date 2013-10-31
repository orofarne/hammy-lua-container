#pragma once

#include "Context.hpp"

#include "types.hpp"

namespace hammy {

class Worker {
    public:
        Worker(Context &c);
        ~Worker() throw();

        Buffer operator()(char *in_buf, size_t in_size);

    private:
        Context &c_;
};

}
