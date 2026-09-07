#include <benchmark/benchmark.h>
#include <fmt/core.h>
#include <mini_fmt/mini_fmt.h>

#include <cstdio>
#include <sstream>
#include <string>
#include <vector>

// 1. 全局分配计数器
struct MallocTracker {
  static inline std::atomic<std::size_t> alloc_count{0};
  static inline std::atomic<std::size_t> alloc_bytes{0};

  static void reset() {
    alloc_count = 0;
    alloc_bytes = 0;
  }
};

;

// 2. 重载全局 operator new/delete 捕获内存分配
void* operator new(std::size_t size) {
  MallocTracker::alloc_count.fetch_add(1, std::memory_order_relaxed);
  MallocTracker::alloc_bytes.fetch_add(size, std::memory_order_relaxed);
  void* ptr = std::malloc(size);
  if (!ptr) throw std::bad_alloc();
  return ptr;
}

void operator delete(void* ptr) noexcept { std::free(ptr); }

void operator delete(void* ptr, std::size_t) noexcept { std::free(ptr); }

// 1. std::stringstream 性能测试
static void BM_StringStream(benchmark::State& state) {
  int val = 42;

  // 记录开始时的分配次数
  std::size_t start_allocs = MallocTracker::alloc_count.load();

  for (auto _ : state) {
    std::ostringstream ss;
    ss << "Value is " << val << " and status is ok";
    std::string res = ss.str();
    benchmark::DoNotOptimize(res);
  }

  // 计算平均每次迭代发生的 new 次数
  std::size_t total_allocs = MallocTracker::alloc_count.load() - start_allocs;
  state.counters["Allocs/op"] =
      static_cast<double>(total_allocs) / state.iterations();
}
BENCHMARK(BM_StringStream);

// 2. snprintf 性能测试
static void BM_Snprintf(benchmark::State& state) {
  int val = 42;
  std::size_t start_allocs = MallocTracker::alloc_count.load();
  char buf[128];
  for (auto _ : state) {
    int len = snprintf(buf, sizeof(buf), "Value is %d  and status is ok", val);
    std::string res(buf, len);
    benchmark::DoNotOptimize(res);
  }
  std::size_t total_allocs = MallocTracker::alloc_count.load() - start_allocs;
  state.counters["Allocs/op"] =
      static_cast<double>(total_allocs) / state.iterations();
}
BENCHMARK(BM_Snprintf);

// 3. mini_fmt 性能测试（SBO 命中，零堆分配）
static void BM_MiniFmt(benchmark::State& state) {
  int val = 42;

  std::size_t start_allocs = MallocTracker::alloc_count.load();

  for (auto _ : state) {
    std::string res = mini_fmt::format("Value is {}  and status is ok", val);
    benchmark::DoNotOptimize(res);
  }

  std::size_t total_allocs = MallocTracker::alloc_count.load() - start_allocs;
  state.counters["Allocs/op"] =
      static_cast<double>(total_allocs) / state.iterations();
}
BENCHMARK(BM_MiniFmt);

// 3. mini_fmt 性能测试（SBO 命中，零堆分配）
static void BM_Fmt(benchmark::State& state) {
  int val = 42;

  std::size_t start_allocs = MallocTracker::alloc_count.load();

  for (auto _ : state) {
    std::string res = fmt::format("Value is {}  and status is ok", val);
    benchmark::DoNotOptimize(res);
  }

  std::size_t total_allocs = MallocTracker::alloc_count.load() - start_allocs;
  state.counters["Allocs/op"] =
      static_cast<double>(total_allocs) / state.iterations();
}
BENCHMARK(BM_Fmt);

BENCHMARK_MAIN();