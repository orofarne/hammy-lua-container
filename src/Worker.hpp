#pragma once

#include "Context.hpp"

namespace hammy {

class Worker {
    public:
        Worker(Context &c);
        ~Worker() throw();

        void *operator()(void *in_buf, size_t in_size, size_t *out_size);

    private:
        Context &c_;
};

}
