#include <gtest/gtest.h>

#include "Context.hpp"

#include <boost/lexical_cast.hpp>

TEST(Context, Empty) {
    using namespace hammy;

    Context c;
}

TEST(Context, Hello) {
    using namespace hammy;

    std::string code = "x = 'hello'\n";

    Context c;
    c.load(code.c_str(), code.length());

    std::string str = c["x"]->asString();
    EXPECT_TRUE( str == "hello" );
}


TEST(Context, GetSetString) {
    using namespace hammy;

    Context c;

    c.setString("a", "hello");
    c.setString("b", "world", 5);

    std::string code = "x = a .. ' ' .. b\n";
    c.load(code.c_str(), code.length());

    std::string str = c["x"]->asString();
    EXPECT_TRUE( str == "hello world" );

    size_t len = 0;
    c["x"]->asString(&len);
    EXPECT_EQ(str.length(), len);
}


TEST(Context, LoadModule) {
    using namespace hammy;

    Context c;

    std::string code = "return {x = 'xyz', y = 10}";
    c.loadModule("xyz", code.c_str(), code.length());

    std::string code2 = "str = xyz.x .. xyz.y";
    c.load(code2.c_str(), code2.length());

    std::string str = c["str"]->asString();
    EXPECT_TRUE( str == "xyz10" );
}

TEST(Context, Shadow) {
    using namespace hammy;

    Context c;

    {
        std::string code = "x = 'hello'\n";
        c.load(code.c_str(), code.length());

        std::string str = c["x"]->asString();
        EXPECT_TRUE( str == "hello" );
    }

    {
        std::string code = "x = 'hello2'\n";
        c.load(code.c_str(), code.length());

        std::string str = c["x"]->asString();
        EXPECT_TRUE( str == "hello2" );
    }
}

TEST(Context, FFI) {
    using namespace hammy;

    std::string code =
        "local ffi = require('ffi')\n"
        "ffi.cdef[[\n"
        "   int getpid();\n"
        "]]\n"
        "x = 'my pid is ' .. ffi.C.getpid()\n"
        ;


    Context c;
    c.load(code.c_str(), code.length());

    std::string str = c["x"]->asString();
    std::ostringstream sstr;
    sstr << "my pid is " << getpid();
    EXPECT_TRUE( str == sstr.str() );
}

TEST(Context, BoolFromTable) {
    using namespace hammy;

    std::string code =
        "t = { a = true, b = false }\n"
        ;


    Context c;
    c.load(code.c_str(), code.length());

    EXPECT_TRUE( c["t"]->asTable()->get("a")->asBool() );
    EXPECT_FALSE( c["t"]->asTable()->get("b")->asBool() );
    EXPECT_FALSE( c["t"]->asTable()->get("c")->asBool() );
}

TEST(Context, IsNil) {
    using namespace hammy;

    std::string code =
        "t = { a = true, b = false }\n"
        ;


    Context c;
    c.load(code.c_str(), code.length());

    EXPECT_TRUE( c["foo"]->isNil() );
    EXPECT_FALSE( c["t"]->isNil() );

    EXPECT_FALSE( c["t"]->asTable()->get("a")->isNil() );
    EXPECT_FALSE( c["t"]->asTable()->get("b")->isNil() );
    EXPECT_TRUE( c["t"]->asTable()->get("c")->isNil() );
}

TEST(Context, Integer) {
    using namespace hammy;

    std::string code =
        "x = 10\n"
        "y = 3.14\n"
        "z = 'hello'"
        ;


    Context c;
    c.load(code.c_str(), code.length());

    bool isnum = false;

    EXPECT_EQ(10, c["x"]->asInteger(&isnum));
    EXPECT_TRUE(isnum);

    EXPECT_EQ(3, c["y"]->asInteger(&isnum));
    EXPECT_TRUE(isnum);

    EXPECT_EQ(0, c["z"]->asInteger(&isnum));
    EXPECT_FALSE(isnum);

    EXPECT_EQ(0, c["foo"]->asInteger(&isnum));
    EXPECT_FALSE(isnum);
}

TEST(Context, Number) {
    using namespace hammy;

    std::string code =
        "x = 10\n"
        "y = 3.14\n"
        "z = 'hello'"
        ;


    Context c;
    c.load(code.c_str(), code.length());

    bool isnum = false;

    EXPECT_DOUBLE_EQ(10, c["x"]->asNumber(&isnum));
    EXPECT_TRUE(isnum);

    EXPECT_DOUBLE_EQ(3.14, c["y"]->asNumber(&isnum));
    EXPECT_TRUE(isnum);

    EXPECT_DOUBLE_EQ(0, c["z"]->asNumber(&isnum));
    EXPECT_FALSE(isnum);

    EXPECT_DOUBLE_EQ(0, c["foo"]->asNumber(&isnum));
    EXPECT_FALSE(isnum);
}
