#pragma once

#include "Module.hpp"
#include "Context.hpp"
#include "ProcessPool.hpp"
#include "Application.hpp"
#include "Timer.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace hammy {

class Manager {
    public:
        Manager(Application &app);
        ~Manager() throw();

        void run();
        void preload(const char *buf, size_t size);

    private:
        Application &app_;
        Timer timer_;

        // Boost::Asio staff
        boost::asio::io_service &io_service_;
        boost::asio::deadline_timer scheduler_timer_;


        Context c_;
        std::shared_ptr<ProcessPool> pp_;
        std::vector<Module> modules_;
        std::multimap<std::string, std::string> dependencies_;

    private:
        void requestCallback(std::shared_ptr<Request> req,
                             std::shared_ptr<Response> res);
        void busCallback(std::string host, std::string metric,
                            Value value, time_t timestamp);
};

}
