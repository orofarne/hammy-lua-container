#include <gtest/gtest.h>

#if 0

#include "ProcessPool.hpp"

#include <iostream>

using namespace hammy;

TEST(ProcessPool, Create) {
    boost::asio::io_service io_service;
    Context c{};
    ProcessPool pp{io_service, c, 1, 10};
}

TEST(ProcessPool, Test1) {
    boost::asio::io_service io_service;

    Context c{};
    std::string code = "return {\n"
    "   onData = function(this, value, timestamp)\n"
    "       this.x = (this.x or 0) + value + 2\n"
    "       return this.x\n"
    "   end\n"
    "}\n";
    c.loadModule("mymodule", code.c_str(), code.length());

    ProcessPool pp{io_service, c, 1, 10};

    std::string state;
    int k = 0;

    for(int i = 0; i < 30; ++i) {
        std::shared_ptr<Request> req{new Request};
        req->host = "myhost";
        req->func = "onData";
        req->metric = "mymodule";
        req->state = state;
        req->value = Value(Value::Type::Numeric, 5);
        req->timestamp = 1380132909 + 2 * i;

        pp.process(req, [&, i](std::shared_ptr<Response> resp) {
            ASSERT_TRUE((bool)resp);

            EXPECT_TRUE(resp->error.empty());
            if(!resp->error.empty()) {
                FAIL() << "An error returned: " << resp->error;
            }
            EXPECT_EQ(0, resp->timestamp);
            ASSERT_EQ(Value::Type::Numeric, resp->value.type());
            EXPECT_DOUBLE_EQ(7, resp->value.as<double>());
            ASSERT_GT(resp->state.size(), 0);
            state = resp->state;

            if(++k >= 30) {
                io_service.stop();
            }
        });
    }

    io_service.run();

    EXPECT_FALSE(state.empty());
    ASSERT_EQ(30, k);
}

#endif
