#pragma once
#include <format>
#include <stop_token>
#include <atomic>
#include <memory>
#include <string_view>
#include <thread>
#include <utility>
#include <cassert>
#include <cstddef>
#include <cstring>

extern void (*print)(std::string_view message);

class logger {
  static constexpr std::size_t capacity = 8192; // 2 ^ 13
  static constexpr std::size_t mask = capacity - 1;
  static constexpr std::size_t max_size = 256;
  static constexpr std::size_t recycle_batch = 64;

  struct alignas(64) slot {
    std::atomic<std::size_t> seq{ 0 };
    std::size_t len{ 0 };
    char data[max_size];
  };

  std::unique_ptr<slot[]> slots_;

  alignas(64) std::atomic<std::size_t> head_{ 0 };
  alignas(64) std::atomic<std::size_t> tail_pub_{ 0 };
  alignas(64) std::size_t tail_{ 0 };

public:
  logger() :
  slots_(new slot[capacity]) {
    for (std::size_t i = 0; i < capacity; i++) {
      slots_[i].seq.store(i, std::memory_order_relaxed);
    }
  }

  void post(std::string_view message) {
    assert(message.size() <= max_size);
    const std::size_t pos = head_.fetch_add(1, std::memory_order_relaxed);
    while (static_cast<std::ptrdiff_t>(pos - tail_pub_.load(std::memory_order_acquire)) >=
           static_cast<std::ptrdiff_t>(capacity)) {
      std::this_thread::yield();
    }
    slot& s = slots_[pos & mask];
    if (!message.empty()) {
      std::memcpy(s.data, message.data(), message.size());
    }
    s.len = message.size();
    s.seq.store(pos + 1, std::memory_order_release);
  }

  template <class... Args>
  void post(std::format_string<Args...> fmt, Args&&... args) {
    char buf[max_size];
    const auto res = std::format_to_n(buf, max_size, fmt, std::forward<Args>(args)...);
    post(std::string_view(buf, static_cast<std::size_t>(res.out - buf)));
  }

  void run(std::stop_token stop) {
    while (!stop.stop_requested()) {
      slot& s = slots_[tail_ & mask];
      if (s.seq.load(std::memory_order_acquire) == tail_ + 1) {
        print({ s.data, s.len });
        tail_++;
        if (tail_ % recycle_batch == 0) {
          tail_pub_.store(tail_, std::memory_order_release);
        }
      } else {
        std::this_thread::yield();
      }
    }
    while (slots_[tail_ & mask].seq.load(std::memory_order_acquire) == tail_ + 1) {
      slot& s = slots_[tail_ & mask];
      print({ s.data, s.len });
      tail_++;
    }
  }
};
