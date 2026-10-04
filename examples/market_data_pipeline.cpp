#include <spsc/queue.hpp>

#include <cstdint>
#include <iostream>
#include <thread>

struct MarketUpdate {
  MarketUpdate(std::uint64_t sequence, std::int64_t price_ticks,
               std::uint32_t quantity)
      : sequence(sequence), price_ticks(price_ticks), quantity(quantity) {}

  std::uint64_t sequence;
  std::int64_t price_ticks;
  std::uint32_t quantity;
};

int main() {
  constexpr std::uint64_t update_count = 1'000'000;
  spsc::Queue<MarketUpdate> feed(8192);

  std::thread decoder([&] {
    for (std::uint64_t sequence = 0; sequence < update_count; ++sequence) {
      feed.emplace(sequence, 100'000 + static_cast<std::int64_t>(sequence % 50),
                   100);
    }
  });

  std::int64_t last_price = 0;
  for (std::uint64_t expected = 0; expected < update_count; ++expected) {
    while (feed.front() == nullptr) {
    }
    const MarketUpdate &update = *feed.front();
    if (update.sequence != expected) {
      std::cerr << "out-of-order market update\n";
      return 1;
    }
    last_price = update.price_ticks;
    feed.pop();
  }

  decoder.join();
  std::cout << "processed " << update_count
            << " updates; last price=" << last_price << '\n';
}
