#include <benchmark/benchmark.h>
#include <fmt/core.h>
#include <mini_fmt/lut.h>
#include <mini_fmt/mini_fmt.h>

// 1. 只测整数格式化（无format string解析）- 公平对比
static void BM_MiniFmt_IntOnly(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    mini_fmt::string_buffer buf;
    mini_fmt::buffer_appender out(buf);
    mini_fmt::formatter<int> fmt;
    mini_fmt::parse_context ctx("");
    fmt.parse(ctx);
    fmt.format(val, out);
    benchmark::DoNotOptimize(buf.data());
  }
}
BENCHMARK(BM_MiniFmt_IntOnly);

// 1b. 只测LUT格式化核心部分
static void BM_MiniFmt_LUTCore(benchmark::State& state) {
  int val = 42;
  char buf[64];
  for (auto _ : state) {
    char* end = mini_fmt::detail::format_signed_lut_to_buf(val, buf);
    benchmark::DoNotOptimize(end);
  }
}
BENCHMARK(BM_MiniFmt_LUTCore);

// 1c. 只测buffer append
static void BM_MiniFmt_BufferAppend(benchmark::State& state) {
  const char* data = "42";
  size_t len = 2;
  for (auto _ : state) {
    mini_fmt::string_buffer buf;
    mini_fmt::buffer_appender out(buf);
    out.append(data, len);
    benchmark::DoNotOptimize(buf.data());
  }
}
BENCHMARK(BM_MiniFmt_BufferAppend);

static void BM_Fmt_IntOnly(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    std::string res = fmt::format("{}", val);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_Fmt_IntOnly);

// 2. 只测字符串拼接（无参数）
static void BM_MiniFmt_StringOnly(benchmark::State& state) {
  for (auto _ : state) {
    std::string res = mini_fmt::format("Value is ");
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_MiniFmt_StringOnly);

static void BM_Fmt_StringOnly(benchmark::State& state) {
  for (auto _ : state) {
    std::string res = fmt::format("Value is ");
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_Fmt_StringOnly);

// 3. 测format_arg构造开销
static void BM_MiniFmt_FormatArgConstruct(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    mini_fmt::format_arg arg(val);
    benchmark::DoNotOptimize(arg);
  }
}
BENCHMARK(BM_MiniFmt_FormatArgConstruct);

// 4. 测vformat_to解析开销
static void BM_MiniFmt_ParseOnly(benchmark::State& state) {
  for (auto _ : state) {
    mini_fmt::string_buffer buf;
    mini_fmt::buffer_appender out(buf);
    mini_fmt::format_arg_store<0> store{};
    mini_fmt::format_args args(store.data, 0);
    mini_fmt::detail::vformat_to(out, "Value is ", args);
    benchmark::DoNotOptimize(buf.data());
  }
}
BENCHMARK(BM_MiniFmt_ParseOnly);

// 5. 完整流程对比
static void BM_MiniFmt_Full(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    std::string res = mini_fmt::format("Value is {}", val);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_MiniFmt_Full);

static void BM_Fmt_Full(benchmark::State& state) {
  int val = 42;
  for (auto _ : state) {
    std::string res = fmt::format("Value is {}", val);
    benchmark::DoNotOptimize(res);
  }
}
BENCHMARK(BM_Fmt_Full);

BENCHMARK_MAIN();
