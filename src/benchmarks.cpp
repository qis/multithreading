// Run on (16 X 2904 MHz CPU s)
// CPU Caches:
//   L1 Data 32 KiB (x8)
//   L1 Instruction 32 KiB (x8)
//   L2 Unified 256 KiB (x8)
//   L3 Unified 16384 KiB (x1)
// -------------------------------------------------------------------------------------------------
// Benchmark                                                       Time             CPU   Iterations
// -------------------------------------------------------------------------------------------------
// string<logger_with_mutex>                                    3144 ns         1059 ns      3525248
// string_literal<logger_with_mutex>                            2906 ns          586 ns      5811888
// string<logger_with_condition_variable>                        593 ns          605 ns      6720000
// string_literal<logger_with_condition_variable>                606 ns          607 ns      5973336
// string<logger_with_semaphore>                                 636 ns          636 ns      5973336
// string_literal<logger_with_semaphore>                         691 ns          681 ns      6720000
// string<logger_with_atomic_wait>                               575 ns          578 ns      7168000
// string_literal<logger_with_atomic_wait>                       583 ns          576 ns      8270768
// string<logger_with_atomic_wait_and_malloc>                    540 ns          540 ns      7964448
// string_literal<logger_with_atomic_wait_and_malloc>            533 ns          532 ns      7168000
// string<logger_with_coroutines>                                571 ns          569 ns      7964448
// string_literal<logger_with_coroutines>                        507 ns          511 ns      8960000
// string<logger_with_coroutines_fast>                           614 ns          621 ns      7168000
// string_literal<logger_with_coroutines_fast>                   587 ns          597 ns      7168000

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

template <class T>
static void string(benchmark::State& state) {
  auto& logger = get_logger<T>();
  const std::string s{"benchmark\n"};
  for (auto _ : state)
    logger.post(s);
}

template <class T>
static void string_literal(benchmark::State& state) {
  auto& logger = get_logger<T>();
  for (auto _ : state)
    logger.post("benchmark\n");
}

constexpr double seconds = 3.0;

#ifdef logger
BENCHMARK(string<logger>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_mutex
BENCHMARK(string<logger_with_mutex>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_mutex>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_condition_variable
BENCHMARK(string<logger_with_condition_variable>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_condition_variable>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_semaphore
BENCHMARK(string<logger_with_semaphore>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_semaphore>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_atomic_wait
BENCHMARK(string<logger_with_atomic_wait>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_atomic_wait>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_atomic_wait_and_malloc
BENCHMARK(string<logger_with_atomic_wait_and_malloc>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_atomic_wait_and_malloc>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_coroutines
BENCHMARK(string<logger_with_coroutines>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_coroutines>)->Threads(8)->MinTime(seconds);
#endif

#ifdef logger_with_coroutines_fast
BENCHMARK(string<logger_with_coroutines_fast>)->Threads(8)->MinTime(seconds);
BENCHMARK(string_literal<logger_with_coroutines_fast>)->Threads(8)->MinTime(seconds);
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

  std::list<std::jthread> threads;
#ifdef logger
  threads.emplace_back(create_logger<logger>());
#endif
#ifdef logger_with_mutex
  threads.emplace_back(create_logger<logger_with_mutex>());
#endif
#ifdef logger_with_condition_variable
  threads.emplace_back(create_logger<logger_with_condition_variable>());
#endif
#ifdef logger_with_semaphore
  threads.emplace_back(create_logger<logger_with_semaphore>());
#endif
#ifdef logger_with_atomic_wait
  threads.emplace_back(create_logger<logger_with_atomic_wait>());
#endif
#ifdef logger_with_atomic_wait_and_malloc
  threads.emplace_back(create_logger<logger_with_atomic_wait_and_malloc>());
#endif
#ifdef logger_with_coroutines
  threads.emplace_back(create_logger<logger_with_coroutines>());
#endif
#ifdef logger_with_coroutines_fast
  threads.emplace_back(create_logger<logger_with_coroutines_fast>());
#endif

  reporter reporter;
  benchmark::RunSpecifiedBenchmarks(&reporter);
  for (auto& thread : threads) {
    thread.request_stop();
    thread.join();
  }
}
