// iterations:100000/threads:16
// Run on (16 X 2357.63 MHz CPU s)
// CPU Caches:
//   L1 Data 32 KiB (x8)
//   L1 Instruction 32 KiB (x8)
//   L2 Unified 512 KiB (x8)
//   L3 Unified 16384 KiB (x1)
// Load Average: 1.54, 1.13, 1.62
// -------------------------------------------------------------------------------------------------
// Benchmark                                                       Time             CPU   Iterations
// -------------------------------------------------------------------------------------------------
// string<logger_with_mutex>                                    4538 ns         2347 ns      1600000
// string_literal<logger_with_mutex>                            4522 ns         2473 ns      1600000
// string<logger_with_condition_variable>                       2838 ns         1037 ns      1600000
// string_literal<logger_with_condition_variable>               2830 ns         1007 ns      1600000
// string<logger_with_semaphore>                                2846 ns          994 ns      1600000
// string_literal<logger_with_semaphore>                        2801 ns         1006 ns      1600000
// string<logger_with_atomic_wait>                              3154 ns         1654 ns      1600000
// string_literal<logger_with_atomic_wait>                      3188 ns         1654 ns      1600000
// string<logger_with_atomic_wait_and_malloc>             [!]   1567 ns          405 ns      1600000
// string_literal<logger_with_atomic_wait_and_malloc>     [!]   1569 ns          375 ns      1600000
// string<logger_with_coroutines>                               4691 ns         2191 ns      1600000
// string_literal<logger_with_coroutines>                       3188 ns         1627 ns      1600000
// string<logger_with_coroutines_fast>                          3085 ns         1666 ns      1600000
// string_literal<logger_with_coroutines_fast>                  1861 ns          981 ns      1600000
// string<logger_with_boost_lockfree_queue>                     4959 ns         4365 ns      1600000
// string_literal<logger_with_boost_lockfree_queue>             4888 ns         4461 ns      1600000
// string<logger_with_boost_asio>                               5113 ns         3438 ns      1600000
// string_literal<logger_with_boost_asio>                       4083 ns         2839 ns      1600000
// string<logger_with_tbb_queue>                                3746 ns         2660 ns      1600000
// string_literal<logger_with_tbb_queue>                        2903 ns         2163 ns      1600000
// string<logger_with_tbb_bounded_queue>                       19196 ns         2375 ns      1600000
// string_literal<logger_with_tbb_bounded_queue>               17151 ns         2432 ns      1600000
// string<logger_with_tbb_scalable_allocator>                   1498 ns         1286 ns      1600000
// string_literal<logger_with_tbb_scalable_allocator>           1585 ns         1379 ns      1600000
// string<logger_with_spdlog>                                  14415 ns         3807 ns      1600000
// string_literal<logger_with_spdlog>                          14450 ns         3814 ns      1600000

#include "logger.hpp"
#include <benchmark/benchmark.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

template <class Logger>
std::shared_ptr<Logger> get_logger() {
  static std::weak_ptr<Logger> wp;
  auto sp = wp.lock();
  if (!sp) {
    static std::mutex mutex;
    const std::lock_guard lock{mutex};
    sp = wp.lock();
    if (!sp) {
      sp = std::make_shared<Logger>();
      wp = sp;
    }
  }
  return sp;
}

// Make sure we're posting a dynamically allocated string.
static_assert(std::string{}.capacity() < 20);

template <class T>
static void string(benchmark::State& state) {
  auto logger = get_logger<T>();
  const std::string s{"12345678901234567890\n"};
  for (auto _ : state)
    logger->post(s);
}

template <class T>
static void string_literal(benchmark::State& state) {
  auto logger = get_logger<T>();
  for (auto _ : state)
    logger->post("12345678901234567890\n");
}

constexpr int threads = 16;
constexpr benchmark::IterationCount count = 100'000;

#ifdef logger
BENCHMARK(string<logger>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_mutex
BENCHMARK(string<logger_with_mutex>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_mutex>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_condition_variable
BENCHMARK(string<logger_with_condition_variable>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_condition_variable>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_semaphore
BENCHMARK(string<logger_with_semaphore>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_semaphore>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_atomic_wait
BENCHMARK(string<logger_with_atomic_wait>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_atomic_wait>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_atomic_wait_and_malloc
BENCHMARK(string<logger_with_atomic_wait_and_malloc>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_atomic_wait_and_malloc>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_coroutines
BENCHMARK(string<logger_with_coroutines>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_coroutines>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_coroutines_fast
BENCHMARK(string<logger_with_coroutines_fast>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_coroutines_fast>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_boost_lockfree_queue
BENCHMARK(string<logger_with_boost_lockfree_queue>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_boost_lockfree_queue>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_boost_asio
BENCHMARK(string<logger_with_boost_asio>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_boost_asio>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_tbb_queue
BENCHMARK(string<logger_with_tbb_queue>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_tbb_queue>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_tbb_bounded_queue
BENCHMARK(string<logger_with_tbb_bounded_queue>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_tbb_bounded_queue>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_tbb_scalable_allocator
BENCHMARK(string<logger_with_tbb_scalable_allocator>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_tbb_scalable_allocator>)->Threads(threads)->Iterations(count);
#endif

#ifdef logger_with_spdlog
BENCHMARK(string<logger_with_spdlog>)->Threads(threads)->Iterations(count);
BENCHMARK(string_literal<logger_with_spdlog>)->Threads(threads)->Iterations(count);
#endif

template <class T>
std::jthread create_logger() {
  std::binary_semaphore started{0};
  std::jthread thread{[semaphore = &started](std::stop_token stop) {
    semaphore->release();
    get_logger<T>()->run(stop);
  }};
  started.acquire();
  return thread;
}

class reporter : public benchmark::ConsoleReporter {
public:
  reporter() : stdout_{dup(fileno(stdout))} {
    if (stdout_ == -1) {
      throw std::system_error{
        std::error_code{errno, std::system_category()},
        "Could not duplicate stdout handle"
      };
    }
    auto null = freopen(
#ifdef _WIN32
      "NUL",
#else
      "/dev/null",
#endif
      "w", stdout);
  }

  reporter(reporter&& other) = delete;
  reporter(const reporter& other) = delete;
  reporter& operator=(reporter&& other) = delete;
  reporter& operator=(const reporter& other) = delete;

  ~reporter() {
    fflush(stdout);
    (void)dup2(stdout_, fileno(stdout));
    close(stdout_);
    benchmark::ConsoleReporter::ReportRuns(reports_);
  }

  void ReportRuns(const std::vector<Run>& reports) override {
    reports_.insert(end(reports_), begin(reports), end(reports));
  }

private:
  int stdout_;
  std::vector<Run> reports_;
};

int main(int argc, char** argv) {
  benchmark::Initialize(&argc, argv);
  if (benchmark::ReportUnrecognizedArguments(argc, argv))
    return EXIT_FAILURE;

  std::list<std::jthread> logger_threads;
#ifdef logger
  logger_threads.emplace_back(create_logger<logger>());
#endif
#ifdef logger_with_mutex
  logger_threads.emplace_back(create_logger<logger_with_mutex>());
#endif
#ifdef logger_with_condition_variable
  logger_threads.emplace_back(create_logger<logger_with_condition_variable>());
#endif
#ifdef logger_with_semaphore
  logger_threads.emplace_back(create_logger<logger_with_semaphore>());
#endif
#ifdef logger_with_atomic_wait
  logger_threads.emplace_back(create_logger<logger_with_atomic_wait>());
#endif
#ifdef logger_with_atomic_wait_and_malloc
  logger_threads.emplace_back(create_logger<logger_with_atomic_wait_and_malloc>());
#endif
#ifdef logger_with_coroutines
  logger_threads.emplace_back(create_logger<logger_with_coroutines>());
#endif
#ifdef logger_with_coroutines_fast
  logger_threads.emplace_back(create_logger<logger_with_coroutines_fast>());
#endif
#ifdef logger_with_boost_lockfree_queue
  logger_threads.emplace_back(create_logger<logger_with_boost_lockfree_queue>());
#endif
#ifdef logger_with_boost_asio
  logger_threads.emplace_back(create_logger<logger_with_boost_asio>());
#endif
#ifdef logger_with_tbb_queue
  logger_threads.emplace_back(create_logger<logger_with_tbb_queue>());
#endif
#ifdef logger_with_tbb_bounded_queue
  logger_threads.emplace_back(create_logger<logger_with_tbb_bounded_queue>());
#endif
#ifdef logger_with_tbb_scalable_allocator
  logger_threads.emplace_back(create_logger<logger_with_tbb_scalable_allocator>());
#endif

  reporter reporter;
  benchmark::RunSpecifiedBenchmarks(&reporter);
  for (auto& thread : logger_threads) {
    thread.request_stop();
    thread.join();
  }
}
