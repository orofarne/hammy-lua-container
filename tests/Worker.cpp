#include <gtest/gtest.h>

#include "Worker.hpp"
#include "Encode.hpp"

using namespace hammy;

TEST(Worker, Create) {
    Context c{};
    Worker w{c};
}

TEST(Worker, Test1) {
    Context c{};

    std::string code = "return {\n"
    "   onData = function(this, value, timestamp)\n"
    "       this.x = (this.x or 0) + value + 2\n"
    "       return this.x\n"
    "   end\n"
    "}\n";
    c.loadModule("mymodule", code.c_str(), code.length());

    Worker w{c};

    std::string state;

    for(int i = 0; i < 10; ++i) {
        Request req;
        req.module = "mymodule";
        req.func = "onData";
        req.metric = "test_metric";
        req.state = state;
        req.value = Value(Value::Type::Numeric, 5);
        req.timestamp = 1380132909 + 2 * i;

        std::string req_bin = encodeRequest(req);
        ASSERT_GT(req_bin.size(), 0);

        size_t res_bin_len = 0;
        void *res_bin = w(
                reinterpret_cast<void *>(const_cast<char *>(req_bin.data())),
                req_bin.size(),
                &res_bin_len
            );
        ASSERT_GT(res_bin_len, 0);

        std::string buf{reinterpret_cast<char *>(res_bin), res_bin_len};

        std::shared_ptr<Response> resp = decodeResponse(buf);
        ASSERT_TRUE((bool)resp);

        EXPECT_TRUE(resp->error.empty());
        if(!resp->error.empty()) {
            FAIL() << "An error returned: " << resp->error;
        }
        EXPECT_EQ(0, resp->timestamp);
        ASSERT_EQ(Value::Type::Numeric, resp->value.type());
        EXPECT_DOUBLE_EQ((5 + 2) * (i + 1), resp->value.as<double>());
        ASSERT_GT(resp->state.size(), 0);
        state = resp->state;
    }
}
