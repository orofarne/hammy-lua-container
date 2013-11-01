#include <gtest/gtest.h>

#include "ProcessPool.hpp"

#include <iostream>

#include <boost/format.hpp>

#define DEBUG_MSG 1

using namespace hammy;

static Buffer echo_f(char *in_buf, size_t in_size) {
    return Buffer{ new std::string{in_buf, in_size} };
}

TEST(ProcessPool, Create) {
    boost::asio::io_service io_service;
    ProcessPool pp{io_service, &echo_f, 2, 10};
}

TEST(ProcessPool, CheckBoostFormat) {
    std::string s = str(boost::format("x = %1$02d") % 3);
    ASSERT_EQ(6, s.length());
    EXPECT_TRUE(s == "x = 03");

    std::string s2 = str(boost::format("x = %1$02d") % 99);
    ASSERT_EQ(6, s2.length());
    EXPECT_TRUE(s2 == "x = 99");
}

TEST(ProcessPool, Test1) {
    boost::asio::io_service io_service;
    ProcessPool pp{io_service, &echo_f, 2, 10};

    int n = 0;
    int N = 100;

    for(int i = 0; i < N; ++i) {
        std::string s = str(boost::format("\xa5i=%1$03d") % i);
        ASSERT_EQ(6, s.length());
        Buffer b{new std::string(s)};

        pp.process(b, [&n, i, N, &io_service](Buffer b, Error e) {
            ASSERT_FALSE(e);

            std::string rs = str(boost::format("\xa5i=%1$03d") % i);
            ASSERT_EQ(rs.length(), b->length());
            for(int j = 0; j < rs.length(); ++j) {
                EXPECT_EQ(rs[j], (*b)[j]);
            }

            ++n;
#if DEBUG_MSG
            std::cerr << "-- [" << n << "]: " << *b << std::endl;
#endif
            if(n >= N) {
                io_service.stop();
            }
        });
    }

    io_service.run();

    EXPECT_EQ(N, n);
}
