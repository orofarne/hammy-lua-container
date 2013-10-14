#pragma once

#include "Application.hpp"

namespace hammy {

class Timer {
    public:
        Timer(Application &app);
        ~Timer() throw();

    private:
        Application &app_;
};

}
