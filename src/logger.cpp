#include "logger.hpp"

void print(std::string_view text) {
  static std::mutex mutex;
  const std::lock_guard lock{mutex};
  std::fwrite(text.data(), text.size(), sizeof(char), stdout);
  std::fputc('\n', stdout);
  std::fflush(stdout);
}

void logger::post(std::string text) noexcept {
  if (closing_.load(std::memory_order_relaxed))
    return;
  {
    const std::lock_guard<std::mutex> lock(m_);
    if (closing_.load(std::memory_order_relaxed))
      return;
    queue_.emplace_back(std::move(text));
  }
  cv_.notify_one();
}


void logger::run(std::stop_token st) {
  std::stop_callback on_stop{st, [this] {
                               cv_.notify_all();
                             }};
  std::unique_lock<std::mutex> lock(m_);
  while (true) {
    cv_.wait(lock, [this, &st] {
      return !queue_.empty() || closing_.load(std::memory_order_relaxed) || st.stop_requested();
    });
    if (queue_.empty() && (closing_.load(std::memory_order_relaxed) || st.stop_requested()))
      break;
    auto batch = std::exchange(queue_, {});
    lock.unlock();
    for (const auto& e : batch)
      std::fputs(e.c_str(), stdout);
    std::fflush(stdout);
    lock.lock();
  }
}
