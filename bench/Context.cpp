#include <celero/Celero.h>

#include "Context.hpp"

#include <memory>

using namespace hammy;

class ContextBenchFixture : public celero::TestFixture {
    public:
        virtual void SetUp() {
            c.reset(new Context);
        }

        virtual void TearDown() {
            c.reset();
        }

        std::shared_ptr<Context> c;
};

BASELINE_F(ContextBenchTest, Baseline, ContextBenchFixture, 10, 1000) {
    std::string code = "x = 'hello'\n";
    celero::DoNotOptimizeAway(code);
}

BENCHMARK_F(ContextBenchTest, Hello, ContextBenchFixture, 10, 1000) {
    std::string code = "x = 'hello'\n";

    c->load(code.c_str(), code.length());

    std::string str = (*c)["x"]->asString();
    celero::DoNotOptimizeAway(str);
}
