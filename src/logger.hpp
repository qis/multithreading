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
  // Queues the message. Called from multiple threads.
  void post(std::string text) {
    queue_.emplace_back(std::move(text));
  }

  // Processes messages. Called from a single thread.
  void run(std::stop_token stop) {
    while (true) {
      while (queue_.empty()) {
        if (stop.stop_requested())
          return;
        std::this_thread::sleep_for(100ms);
      }
      for (const auto& e : std::exchange(queue_, {}))
        std::fputs(e.data(), stdout);
      std::fflush(stdout);
    }
  }

private:
  std::deque<std::string> queue_;
};

#define logger logger
