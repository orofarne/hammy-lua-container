#include <celero/Celero.h>

#include "SubProcess.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

using namespace hammy;

static Buffer echo_f(char *in_buf, size_t in_size) {
    return Buffer{ new std::string{in_buf, in_size} };
}

class SubProcessBenchFixture : public celero::TestFixture {
    public:
        virtual void SetUp() {
            io_service.reset(new boost::asio::io_service);
            sp.reset(new SubProcess{*io_service, echo_f});
            sp->fork();
        }

        virtual void TearDown() {
            sp.reset();
            io_service.reset();

            // int status;
            // pid_t pid = ::wait(&status);
            // assert(pid > 0);
        }

        std::shared_ptr<boost::asio::io_service> io_service;
        std::shared_ptr<SubProcess> sp;
};

BASELINE_F(SubProcessBenchTest, Baseline, SubProcessBenchFixture, 10, 1000) {
    Buffer b{ new std::string{"\xa5Hello"} };
    Buffer rb = echo_f(const_cast<char *>(b->data()), b->size());
    celero::DoNotOptimizeAway(rb);
}

BENCHMARK_F(SubProcessBenchTest, Echo, SubProcessBenchFixture, 10, 1000) {
    Buffer b{ new std::string{"\xa5Hello"} };
    sp->process(b, [&](Buffer rb, Error e) {
        assert(!e);
        assert(rb);

        io_service->stop();
    });

    io_service->run();
}
