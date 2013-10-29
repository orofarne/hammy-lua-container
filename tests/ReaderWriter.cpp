#include <gtest/gtest.h>

#include "Reader.hpp"
#include "Writer.hpp"

#include <array>
#include <stdexcept>
#include <iostream>
#include <boost/asio.hpp>

using namespace hammy;

class TestReaderWriter : public ::testing::Test {
    protected:
        void SetUp() {
            io = new boost::asio::io_service;
            d0 = new boost::asio::posix::stream_descriptor{*io};
            d1 = new boost::asio::posix::stream_descriptor{*io};

            if(pipe(pipefd.data()) < 0)
                throw std::runtime_error("pipe");

            for(int fd : pipefd)
                if(fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
                    throw std::runtime_error("fcntl");

            d0->assign(pipefd[0]);
            d1->assign(pipefd[1]);
        }

        void TearDown() {
            delete d0;
            delete d1;
            delete io;
        }

        boost::asio::io_service *io;
        boost::asio::posix::stream_descriptor *d0;
        boost::asio::posix::stream_descriptor *d1;

        std::array<int, 2> pipefd;
};

TEST_F(TestReaderWriter, CreateReader) {
    Reader<decltype(*d0)> r{*io, *d0, [&](Error e) {
        ASSERT_FALSE(e);
    }};
}

TEST_F(TestReaderWriter, CreateWriter) {
    Writer<decltype(*d1)> w{*io, *d1, [&](Error e) {
        ASSERT_FALSE(e);
    }};
}

TEST_F(TestReaderWriter, Hello_x10) {
    int i_res = 0;

    Reader<decltype(*d0)> r{*io, *d0, [&](Error e) {
        if(e)
            FAIL() << "Error: " << e->what();

        size_t len;
        char *ptr = r.get(&len);
        ASSERT_GT(len, 0);
        ASSERT_NE(nullptr, ptr);

        std::string buf(ptr, len);
        r.pop();
        std::string test = "\x91";
        test += std::to_string(i_res)[0];

        ++i_res;
        if(i_res >= 10)
            io->stop();

        if(buf != test)
            FAIL() << "(buf != test): " << buf << " != " << test;
    }};

    Writer<decltype(*d1)> w{*io, *d1, [&](Error e) {
        if(e)
            FAIL() << "Error: " << e->what();
    }};

    for(int i = 0; i < 10; ++i) {
        // 10100001 -> 0x91
        char buf[] = {'\x91', std::to_string(i)[0]};
        w.write(::strdup(buf), sizeof(buf));
    }

    io->run();
}
