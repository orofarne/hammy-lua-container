#include <gtest/gtest.h>

#include "Reader.hpp"

#include <array>
#include <stdexcept>
#include <boost/asio.hpp>

using namespace hammy;

class TestReader : public ::testing::Test {
    protected:
        void SetUp() {
            io = new boost::asio::io_service;
            d = new boost::asio::posix::stream_descriptor{*io};

            if(pipe(pipefd.data()) < 0)
                throw std::runtime_error("pipe");

            if(fcntl(pipefd[1], F_SETFL, O_NONBLOCK) < 0)
                throw std::runtime_error("fcntl");

            d->assign(pipefd[1]);
        }

        void TearDown() {
            delete d;
            delete io;
        }

        boost::asio::io_service *io;
        boost::asio::posix::stream_descriptor *d;

        std::array<int, 2> pipefd;
};

TEST_F(TestReader, Create) {
    Reader<decltype(*d)> r{*io, *d};
}


