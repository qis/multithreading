// min_time:3.000/threads:16
// Run on (16 X 2357.91 MHz CPU s)
// CPU Caches:
//   L1 Data 32 KiB (x8)
//   L1 Instruction 32 KiB (x8)
//   L2 Unified 512 KiB (x8)
//   L3 Unified 16384 KiB (x1)
// Load Average: 1.07, 1.00, 0.61
// -------------------------------------------------------------------------------------------------
// Benchmark                                                       Time             CPU   Iterations
// -------------------------------------------------------------------------------------------------
// string<logger_with_mutex>                                    4777 ns         2717 ns      2153040
// string_literal<logger_with_mutex>                            4683 ns         2591 ns      1618288
// string<logger_with_condition_variable>                       2848 ns         1001 ns      4217616
// string_literal<logger_with_condition_variable>               2845 ns         1002 ns      4203792
// string<logger_with_semaphore>                                2845 ns          982 ns      4224960
// string_literal<logger_with_semaphore>                        2839 ns          977 ns      4314304
// string<logger_with_atomic_wait>                              2867 ns          994 ns      4208896
// string_literal<logger_with_atomic_wait>                      2849 ns          983 ns      4269856
// string<logger_with_atomic_wait_and_malloc>                   1653 ns          412 ns     10171040
// string_literal<logger_with_atomic_wait_and_malloc>           1747 ns          426 ns     10266176
// string<logger_with_coroutines>                               4930 ns         1713 ns      2475920
// string_literal<logger_with_coroutines>                       3265 ns         1178 ns      3582896
// string<logger_with_coroutines_fast>                          3336 ns         1223 ns      3422688
// string_literal<logger_with_coroutines_fast>                  1854 ns          514 ns      8178960

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

#ifdef logger_with_tbb
BENCHMARK(string<logger_with_tbb>)->Threads(threads)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_tbb>)->Threads(threads)->MinTime(seconds);
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
#ifdef logger_with_tbb
  logger_threads.emplace_back(create_logger<logger_with_tbb>());
#endif

  reporter reporter;
  benchmark::RunSpecifiedBenchmarks(&reporter);
  for (auto& thread : logger_threads) {
    thread.request_stop();
    thread.join();
  }
}
