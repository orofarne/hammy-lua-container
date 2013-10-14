#include "Manager.hpp"

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>

#include <sstream>
#include <stdexcept>
#include <limits>

#include <boost/filesystem.hpp>

// Warning! Not thread-safe!
#define THROW_ERRNO(comment) { \
        std::ostringstream msg; \
        msg << "Error " << errno << " " << comment << ": " << strerror(errno); \
        throw std::runtime_error(msg.str()); \
    }

namespace hammy {

Manager::Manager(Application &app)
    : app_(app)
    , timer_(app_)
    , io_service_(app_.ioService())
    , scheduler_timer_(io_service_)
{
    namespace ph=std::placeholders;

    auto h_cfg = app.config().get_child("hammy");

    unsigned int pool_size = h_cfg.get<unsigned int>("pool_size");
    unsigned int worker_lifetime = h_cfg.get<unsigned int>("worker_lifetime");

    pp_.reset(new ProcessPool{io_service_, c_, pool_size, worker_lifetime});

    app.bus().setCallback(
            std::bind(&Manager::busCallback, this,
                        ph::_1, ph::_2, ph::_3, ph::_4)
        );
}

Manager::~Manager() throw() {

}

void
Manager::loadFiles(std::vector<std::string> const &files) {
    namespace fs = boost::filesystem;

    for(std::string const &file : files) {
        int fd = open(file.c_str(), O_RDONLY);
        if(fd < 0)
            THROW_ERRNO("open");

        struct stat statbuf;
        if ( fstat(fd, &statbuf) < 0 )
            THROW_ERRNO("fstat");

        void *data = mmap(0, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
        if (data == MAP_FAILED)
            THROW_ERRNO("mmap");

        fs::path p(file);
        std::string filename = p.filename().native();
        std::string module_name = filename.substr(0, filename.length() - 4);

        if(module_name == "__init__") {
            c_.load((const char *)data, statbuf.st_size);
        }
        else {
            modules_.push_back(Module(module_name));
            c_.loadModule(module_name.c_str(), (const char *)data, statbuf.st_size);
        }

        if (munmap(data, statbuf.st_size))
            THROW_ERRNO("munmap");
    }
}

void
Manager::run() {
    prepareModules();

    io_service_.dispatch(
            std::bind(&Manager::runIter, this, boost::system::error_code{})
            );

    io_service_.run();
}

void
Manager::runIter(const boost::system::error_code &error) {
    std::cerr << __FUNCTION__ << std::endl;

    if(error) {
        throw std::runtime_error(error.message());
    }

    time_t t = 0;
    while(t == 0) {
        t = startSomething();
    }

    if(t == std::numeric_limits<decltype(t)>::max())
        return;

    scheduler_timer_.expires_from_now(boost::posix_time::seconds(t));
    scheduler_timer_.async_wait(
            std::bind(&Manager::runIter, this, std::placeholders::_1)
            );
}

void
Manager::prepareModules() {
    for(Module &m : modules_) {
        if(c_[m.name().c_str()]->isNil())
            continue;

        auto m_table = c_[m.name().c_str()];
        // Disabled
        if(!m_table->asTable()->get("disabled")->isNil() && m_table->asTable()->get("disabled")->asBool())
            m.disable();

        // Periodic
        bool is_num = false;
        time_t t = m_table->asTable()->get("period")->asInteger(&is_num);
        if(is_num)
            m.setPeriod(t);
        std::cerr << "period: " << t << " (" << is_num << ", " << m_table->asTable()->get("period")->isNil() << ")" << std::endl;

        // Dependencies
        if(!m_table->asTable()->get("dependencies")->isNil()) {
            auto d_table = m_table->asTable()->get("dependencies");
            for(unsigned int i = 1; i < std::numeric_limits<decltype(i)>::max(); ++i) {
                if(d_table->asTable()->get((std::to_string(i)).c_str())->isNil())
                    break;

                std::string dep = d_table->asTable()->get((std::to_string(i)).c_str())->asString();

                std::cerr << dep << " -> " << m.name() << std::endl;

                // Check dependency is valid
                if(std::find_if(modules_.cbegin(), modules_.cend(), [&dep](const Module &im) -> bool {
                        return im.name() == dep;
                    }) == modules_.cend())
                {
                    std::ostringstream msg;
                    msg << "Invalid dependency of module '" << m.name()
                        << "': '" << dep << "'";
                    throw std::runtime_error(msg.str());
                }

                dependencies_.insert(std::pair<std::string, std::string>(dep, m.name()));
            }
        }
    }
}

time_t
Manager::startSomething() {
    std::cerr << __FUNCTION__ << std::endl;
    time_t now = ::time(nullptr);
    time_t wait;
    wait = std::numeric_limits<decltype(wait)>::max();

    for(Module &m : modules_) {
        std::cerr << "module " << m.name() << '\t' << m.period() << std::endl;
        if(m.status() == Module::Status::Disabled || m.period() == 0)
            continue;

        std::cerr << "-- line " << __LINE__ << std::endl;
        if(m.nextRun() <= now) {
            if(m.status() == Module::Status::Wait) {
                startModule(m);
                wait = 0;
            }
            else if(m.period() < wait) {
                wait = m.period();
            }
        }
        else {
            time_t d = m.nextRun() - now;
            if(wait > d)
                wait = d;
        }
    }
    std::cerr << "wait = " << wait << std::endl;

    return wait;
}

void
Manager::startModule(Module &m, Value v, time_t ts) {
    std::cerr << __FUNCTION__ << std::endl;

    namespace ph=std::placeholders;

    m.start();

    std::shared_ptr<Request> r{new Request};
    r->func = (v.type() == Value::Type::Nil ? "onTimer" : "onData");
    app_.stateKeeper().get("localhost", m.name(), &r->state); // FIXME
    r->host = "localhost"; // FIXME
    r->metric = m.name();
    r->value = v;
    r->timestamp = (ts == 0 ? ::time(nullptr) : ts);

    pp_->process(r, std::bind(&Manager::requestCallback, this, r, ph::_1));
}

void
Manager::requestCallback(std::shared_ptr<Request> req,
                         std::shared_ptr<Response> res)
{
    // TODO: fix race condition here
    app_.stateKeeper().set(req->host, req->metric, res->state);

    app_.bus().push(req->host, req->metric, res->value,
            (res->timestamp == 0 ? ::time(nullptr) : res->timestamp));

    //m.stop(); FIXME
}

void
Manager::busCallback(std::string host, std::string metric,
        Value value, time_t timestamp)
{
    auto ret = dependencies_.equal_range(metric);
    for(auto it = ret.first; it != ret.second; ++it) {
        auto next_m_it = std::find_if(modules_.begin(), modules_.end(),
                [&it](const Module &im) -> bool {
                    return im.name() == it->second;
                }
            );
        startModule(*next_m_it, value, timestamp);
    }
}

}
