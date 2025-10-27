#include "main.hpp"

// Prints the `text` to stdout, appends '\n' and flushes the stream in a thread-safe manner.
// Should not be used by the final logger implementation. Useful for debugging.
void print(std::string_view text);

// Helper function, that reverses a singly linked list.
template <class T>
constexpr T* reverse(T* head) noexcept {
  T* list = nullptr;
  while (head) {
    const auto next = head->next;
    head->next = list;
    list = head;
    head = next;
  }
  return list;
}

// =================================================================================================
// Multithreading
// =================================================================================================
// Task: Implement `logger` as a multiple producers, singele consumer queue as efficient as you can.

class logger {
public:
  logger() = default;
  ~logger() {
    shutdown();
  }
  logger(const logger&) = delete;
  logger& operator=(const logger&) = delete;

  void post(std::string text) noexcept;

  void run(std::stop_token st);

  void shutdown() {
    closing_.store(true, std::memory_order_relaxed);
    cv_.notify_all();
  }

private:
  std::deque<std::string> queue_;
  std::mutex m_;
  std::condition_variable cv_;
  std::atomic<bool> closing_{false};
};

#define logger logger
