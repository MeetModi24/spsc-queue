#include <spsc/queue.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
  constexpr std::uint64_t messages = 20'000'000;
  spsc::Queue<std::uint64_t> queue(4096);
  std::atomic<bool> ready{false};

  std::thread producer([&] {
    while (!ready.load(std::memory_order_acquire)) {
    }
    for (std::uint64_t value = 0; value < messages; ++value) {
      while (!queue.try_push(value)) {
      }
    }
  });

  const auto start = std::chrono::steady_clock::now();
  ready.store(true, std::memory_order_release);
  for (std::uint64_t expected = 0; expected < messages; ++expected) {
    while (queue.front() == nullptr) {
    }
    if (*queue.front() != expected) {
      std::cerr << "sequence error\n";
      return 1;
    }
    queue.pop();
  }
  producer.join();
  const auto elapsed = std::chrono::steady_clock::now() - start;
  const double seconds = std::chrono::duration<double>(elapsed).count();
  const double rate = static_cast<double>(messages) / seconds;

  std::cout << std::fixed << std::setprecision(2)
            << "messages: " << messages << '\n'
            << "elapsed:  " << seconds << " s\n"
            << "rate:     " << rate / 1'000'000.0 << " M msg/s\n";
}
