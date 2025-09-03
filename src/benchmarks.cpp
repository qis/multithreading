// min_time:3.000/threads:16
// Run on (16 X 2904 MHz CPU s)
// CPU Caches:
//   L1 Data 32 KiB (x8)
//   L1 Instruction 32 KiB (x8)
//   L2 Unified 256 KiB (x8)
//   L3 Unified 16384 KiB (x1)
// -------------------------------------------------------------------------------------------------
// Benchmark                                                       Time             CPU   Iterations
// -------------------------------------------------------------------------------------------------
// string<logger_with_mutex>                                   12957 ns         6098 ns       955728
// string_literal<logger_with_mutex>                           11969 ns         4984 ns       843296
// string<logger_with_condition_variable>                       1009 ns          944 ns      3874592
// string_literal<logger_with_condition_variable>                986 ns          961 ns      4258224
// string<logger_with_semaphore>                                1276 ns         1078 ns      3739824
// string_literal<logger_with_semaphore>                        1243 ns         1091 ns      3909824
// string<logger_with_atomic_wait>                              1071 ns          946 ns      4575312
// string_literal<logger_with_atomic_wait>                      1042 ns          876 ns      4674784
// string<logger_with_atomic_wait_and_malloc>                   1023 ns          815 ns      5120000
// string_literal<logger_with_atomic_wait_and_malloc>           1033 ns          862 ns      4674784
// string<logger_with_coroutines>                               1036 ns          765 ns      4943456
// string_literal<logger_with_coroutines>                       1043 ns          799 ns      4887280
// string<logger_with_coroutines_fast>                          1010 ns          821 ns      5059760
// string_literal<logger_with_coroutines_fast>                   935 ns          800 ns      5000928
// string<logger_with_boost_lockfree_queue>                     2029 ns         1606 ns      2150400
// string_literal<logger_with_boost_lockfree_queue>             2059 ns         1678 ns      2606544
// string<logger_with_boost_asio>                               3339 ns         2423 ns      1592896
// string_literal<logger_with_boost_asio>                       3396 ns         2393 ns      1403824
// string<logger_with_tbb_queue>                                2174 ns          888 ns      4344240
// string_literal<logger_with_tbb_queue>                        2218 ns          895 ns      3525248
// string<logger_with_tbb_bounded_queue>                      174394 ns         2344 ns       160000
// string_literal<logger_with_tbb_bounded_queue>              175004 ns         1660 ns       160000

#include "logger.hpp"
#include <benchmark/benchmark.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

template <class Logger>
Logger& get_logger() {
  static Logger logger;
  return logger;
}

// Make sure we're posting a dynamically allocated string.
static_assert(std::string{}.capacity() < 20);

template <class T>
static void string(benchmark::State& state) {
  auto& logger = get_logger<T>();
  const std::string s{"12345678901234567890\n"};
  for (auto _ : state)
    logger.post(s);
}

template <class T>
static void string_literal(benchmark::State& state) {
  auto& logger = get_logger<T>();
  for (auto _ : state)
    logger.post("12345678901234567890\n");
}

constexpr int threads = 16;
constexpr double seconds = 3.0;

#ifdef logger
BENCHMARK(string<logger>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_mutex
BENCHMARK(string<logger_with_mutex>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_mutex>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_condition_variable
BENCHMARK(string<logger_with_condition_variable>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_condition_variable>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_semaphore
BENCHMARK(string<logger_with_semaphore>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_semaphore>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_atomic_wait
BENCHMARK(string<logger_with_atomic_wait>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_atomic_wait>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_atomic_wait_and_malloc
BENCHMARK(string<logger_with_atomic_wait_and_malloc>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_atomic_wait_and_malloc>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_coroutines
BENCHMARK(string<logger_with_coroutines>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_coroutines>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_coroutines_fast
BENCHMARK(string<logger_with_coroutines_fast>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_coroutines_fast>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_boost_lockfree_queue
BENCHMARK(string<logger_with_boost_lockfree_queue>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_boost_lockfree_queue>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_boost_asio
BENCHMARK(string<logger_with_boost_asio>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_boost_asio>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_tbb_queue
BENCHMARK(string<logger_with_tbb_queue>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_tbb_queue>)->Threads(threads)->MinTime(seconds);
#endif

#ifdef logger_with_tbb_bounded_queue
BENCHMARK(string<logger_with_tbb_bounded_queue>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_tbb_bounded_queue>)->Threads(threads)->MinTime(seconds);
#endif

template <class T>
std::jthread create_logger() {
  std::binary_semaphore started{0};
  std::jthread thread{[semaphore = &started](std::stop_token stop) {
    semaphore->release();
    get_logger<T>().run(stop);
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

  reporter reporter;
  benchmark::RunSpecifiedBenchmarks(&reporter);
  for (auto& thread : logger_threads) {
    thread.request_stop();
    thread.join();
  }
}
