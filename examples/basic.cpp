#include <spsc/queue.hpp>

#include <iostream>
#include <thread>

int main() {
  spsc::Queue<int> queue(64);
  std::thread producer([&] {
    for (int value = 1; value <= 1000; ++value) {
      while (!queue.try_push(value)) {
      }
    }
  });

  long total = 0;
  for (int received = 0; received < 1000; ++received) {
    while (queue.front() == nullptr) {
    }
    total += *queue.front();
    queue.pop();
  }
  producer.join();
  std::cout << "sum: " << total << '\n';
}
