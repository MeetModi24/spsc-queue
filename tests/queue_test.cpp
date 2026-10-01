#include <spsc/queue.hpp>

#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <thread>

namespace {

struct Counted {
  static inline std::atomic<int> alive{0};
  explicit Counted(int value) : value(value) { ++alive; }
  Counted(const Counted &other) : value(other.value) { ++alive; }
  ~Counted() noexcept { --alive; }
  int value;
};

void basic_operations() {
  spsc::Queue<int> queue(2);
  assert(queue.capacity() == 2);
  assert(queue.empty());
  assert(queue.try_push(10));
  assert(queue.try_emplace(20));
  assert(!queue.try_push(30));
  assert(queue.size() == 2);
  assert(*queue.front() == 10);
  queue.pop();
  assert(*queue.front() == 20);
  queue.pop();
  assert(queue.empty());
}

void object_lifetime() {
  {
    spsc::Queue<Counted> queue(4);
    assert(queue.try_emplace(7));
    assert(queue.try_emplace(9));
    assert(Counted::alive == 2);
  }
  assert(Counted::alive == 0);
}

void move_only_values() {
  spsc::Queue<std::unique_ptr<int>> queue(1);
  assert(queue.try_push(std::make_unique<int>(42)));
  assert(**queue.front() == 42);
  queue.pop();
}

void concurrent_sequence() {
  constexpr std::uint64_t messages = 1'000'000;
  spsc::Queue<std::uint64_t> queue(1024);
  std::thread producer([&] {
    for (std::uint64_t value = 0; value < messages; ++value) {
      while (!queue.try_push(value)) {
      }
    }
  });

  for (std::uint64_t expected = 0; expected < messages; ++expected) {
    while (queue.front() == nullptr) {
    }
    assert(*queue.front() == expected);
    queue.pop();
  }
  producer.join();
  assert(queue.empty());
}

} // namespace

int main() {
  basic_operations();
  object_lifetime();
  move_only_values();
  concurrent_sequence();
}
