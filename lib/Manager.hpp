#pragma once

#include "Module.hpp"
#include "Context.hpp"
#include "ProcessPool.hpp"
#include "Application.hpp"

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace hammy {

class Manager {
    public:
        Manager(Application &app);
        ~Manager() throw();

        void loadFiles(std::vector<std::string> const &files);
        void run();

    private:
        Application &app_;

        // Boost::Asio staff
        boost::asio::io_service &io_service_;
        boost::asio::deadline_timer scheduler_timer_;


        Context c_;
        std::shared_ptr<ProcessPool> pp_;
        std::vector<Module> modules_;
        std::multimap<std::string, std::string> dependencies_;

    private:
        void runIter(const boost::system::error_code& error);
        void prepareModules();
        time_t startSomething();
        void startModule(Module &m, Value v = Value(), time_t ts = 0);
        void moduleCallback(Module &m, std::shared_ptr<Response> r);
        void busCallback(std::string metric, Value value, time_t timestamp);
};

}
