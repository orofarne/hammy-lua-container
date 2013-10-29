#include <celero/Celero.h>

#include "Context.hpp"

using namespace hammy;

BASELINE(ContextBenchTest, Baseline, 100, 1000) {
    Context c;
    celero::DoNotOptimizeAway(c);
}

BENCHMARK(ContextBenchTest, Hello, 100, 1000) {
    std::string code = "x = 'hello'\n";

    Context c;
    c.load(code.c_str(), code.length());

    std::string str = c["x"]->asString();
    celero::DoNotOptimizeAway(str);
}
