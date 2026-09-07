#include <benchmark/benchmark.h>
#include <mini_fmt/lut.h>
#include <mini_fmt/mini_fmt.h>

// Direct format without temporary objects
static void BM_MiniFmt_DirectFormat(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    char buf[64];
    char* end = mini_fmt::detail::format_signed_lut_to_buf(val, buf);
    std::string res(buf, end - buf);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_MiniFmt_DirectFormat);

// Compare with std::to_chars
static void BM_StdToChars(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    char buf[64];
    auto [ptr, ec] = std::to_chars(buf, buf + sizeof(buf), val);
    std::string res(buf, ptr - buf);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_StdToChars);

BENCHMARK_MAIN();
