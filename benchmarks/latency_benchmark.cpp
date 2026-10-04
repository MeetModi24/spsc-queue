#include <spsc/queue.hpp>

#include <atomic>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
  constexpr std::uint64_t warmup = 100'000;
  constexpr std::uint64_t samples = 2'000'000;
  spsc::Queue<std::uint64_t> requests(64);
  spsc::Queue<std::uint64_t> responses(64);
  std::atomic<bool> ready{false};

  std::thread responder([&] {
    ready.store(true, std::memory_order_release);
    for (std::uint64_t sequence = 0; sequence < warmup + samples;
         ++sequence) {
      while (requests.front() == nullptr) {
      }
      const auto value = *requests.front();
      requests.pop();
      while (!responses.try_push(value)) {
      }
    }
  });

  while (!ready.load(std::memory_order_acquire)) {
  }

  auto exchange = [&](std::uint64_t sequence) {
    while (!requests.try_push(sequence)) {
    }
    while (responses.front() == nullptr) {
    }
    if (*responses.front() != sequence) {
      std::cerr << "sequence error\n";
      std::terminate();
    }
    responses.pop();
  };

  for (std::uint64_t sequence = 0; sequence < warmup; ++sequence) {
    exchange(sequence);
  }

  const auto start = std::chrono::steady_clock::now();
  for (std::uint64_t sequence = warmup; sequence < warmup + samples;
       ++sequence) {
    exchange(sequence);
  }
  const auto elapsed = std::chrono::steady_clock::now() - start;
  responder.join();

  const auto total_ns =
      std::chrono::duration<double, std::nano>(elapsed).count();
  std::cout << std::fixed << std::setprecision(2)
            << "round trips: " << samples << '\n'
            << "average RTT: " << total_ns / samples << " ns\n";
}
