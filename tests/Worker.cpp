#include <gtest/gtest.h>

#include "Worker.hpp"

using namespace hammy;

TEST(Worker, Create) {
    Context c{};
    Worker w{c};
}
