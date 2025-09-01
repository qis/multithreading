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
static void multithreading(benchmark::State& state) {
  auto& logger = get_logger<T>();
  const std::string str{"test"};
  for (auto _ : state)
    logger.post(str);
}

BENCHMARK(multithreading<logger>)->Threads(8);

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

class TestReporter : public benchmark::ConsoleReporter {
public:
  TestReporter() : stdout_{dup(fileno(stdout))} {
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

  TestReporter(TestReporter&& other) = delete;
  TestReporter(const TestReporter& other) = delete;
  TestReporter& operator=(TestReporter&& other) = delete;
  TestReporter& operator=(const TestReporter& other) = delete;

  ~TestReporter() {
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
  threads.emplace_back(create_logger<logger>());

  TestReporter reporter;
  benchmark::RunSpecifiedBenchmarks(&reporter);
  for (auto& thread : threads) {
    thread.request_stop();
    thread.join();
  }
}
