#include "Manager.hpp"

#include <sstream>
#include <stdexcept>
#include <limits>

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
Manager::run() {
    io_service_.run();
}

void
Manager::preload(const char *buf, size_t size) {
    c_.load(buf, size);
}

void
Manager::requestCallback(std::shared_ptr<Request> req,
                         std::shared_ptr<Response> res)
{
    // TODO: fix race condition here
    app_.stateKeeper().set(req->host, req->metric, res->state);

    app_.bus().push(req->host, req->metric, res->value,
            (res->timestamp == 0 ? ::time(nullptr) : res->timestamp));
}

void
Manager::busCallback(std::string host, std::string metric,
        Value value, time_t timestamp)
{
    std::vector<std::string> code = app_.codeLoader().loadTrigger(host, metric);
    // TODO
}

}
