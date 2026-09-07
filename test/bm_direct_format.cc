#include <benchmark/benchmark.h>
#include <fmt/core.h>
#include <mini_fmt/mini_fmt.h>

// Test our format() function directly (like fmt does)
static void BM_MiniFmt_FormatDirect(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    std::string res = mini_fmt::format("{}", val);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_MiniFmt_FormatDirect);

static void BM_Fmt_FormatDirect(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    std::string res = fmt::format("{}", val);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_Fmt_FormatDirect);

BENCHMARK_MAIN();
