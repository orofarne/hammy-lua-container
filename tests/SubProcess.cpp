#include <gtest/gtest.h>

#include "SubProcess.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>

#include <algorithm>
#include <iostream>

#define DEBUG_MSG 0

using namespace hammy;

static Buffer echo_f(char *in_buf, size_t in_size) {
#if DEBUG_MSG
    std::cerr << '[' << in_size << " bytes]:   ";
    std::for_each(
            reinterpret_cast<char *>(in_buf),
            reinterpret_cast<char *>(in_buf) + in_size,
            [](char ch) { std::cerr << " " << ch; }
        );
    std::cerr << std::endl;
#endif

    return Buffer{ new std::string{in_buf, in_size} };
}

TEST(SubProcess, FreeRun) {
    boost::asio::io_service io_service{};
    SubProcess sp{io_service, &echo_f};
}

TEST(SubProcess, Fork) {
    {
        boost::asio::io_service io_service{};
        SubProcess sp{io_service, echo_f};

        sp.fork();

        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100 * 1000000;
        ASSERT_EQ(0, nanosleep(&ts, nullptr));
    }

    int status;
    pid_t pid = ::wait(&status);

    EXPECT_GT(pid, 0);
}

static void echoTest(int N) {
    boost::asio::io_service io_service{};
    std::shared_ptr<SubProcess> sp{ new SubProcess{io_service, echo_f} };

    sp->fork();

    int n = 0;

    Buffer b{ new std::string{"\xa5Hello"} };

    std::function<void(Buffer, Error)> cb = [&](Buffer rb, Error e) {
#if DEBUG_MSG
        std::cerr << "(#" << n << ") [" << rb->size() << " bytes]:   ";
        for(const char ch : *rb) {
            std::cerr << " " << ch;
        }
        std::cerr << std::endl;
#endif

        ++n;
        ASSERT_EQ(b->size(), rb->size());

        for(int i = 0; i < b->size(); ++i) {
            EXPECT_EQ((*b)[i], (*rb)[i]);
        }

        if(n >= N) {
            io_service.stop();
#if DEBUG_MSG
            std::cerr << "-- stop!" << std::endl;
#endif
        }
        else {
            io_service.post(std::bind(&SubProcess::process, sp.get(), b, cb));
        }
    };

    sp->process(b, cb);

    io_service.run();

    sp.reset();

    EXPECT_EQ(N, n);

#if DEBUG_MSG
    std::cerr << "-- waitpid" << std::endl;
#endif
    int status;
    pid_t pid = ::wait(&status);

    EXPECT_GT(pid, 0);
}

TEST(SubProcess, Echo) {
    echoTest(1);
}

TEST(SubProcess, Echo10) {
    echoTest(10);
}

TEST(SubProcess, Echo1000) {
    echoTest(1000);
}
