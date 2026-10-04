#pragma once

#include <atomic>
#include <cassert>
#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace spsc {

// A bounded queue for exactly one producer thread and one consumer thread.
template <class T, class Allocator = std::allocator<T>> class Queue {
  using Traits = std::allocator_traits<Allocator>;
  static constexpr std::size_t cache_line = 64;

  struct alignas(cache_line) ProducerState {
    std::atomic<std::size_t> write{0};
    std::size_t cached_read{0};
  };

  struct alignas(cache_line) ConsumerState {
    std::atomic<std::size_t> read{0};
    std::size_t cached_write{0};
  };

public:
  explicit Queue(std::size_t capacity, const Allocator &allocator = Allocator{})
      : capacity_(capacity == 0 ? 2 : capacity + 1), allocator_(allocator),
        storage_(Traits::allocate(allocator_, capacity_)) {}

  ~Queue() {
    while (front() != nullptr) {
      pop();
    }
    Traits::deallocate(allocator_, storage_, capacity_);
  }

  Queue(const Queue &) = delete;
  Queue &operator=(const Queue &) = delete;
  Queue(Queue &&) = delete;
  Queue &operator=(Queue &&) = delete;

  template <class... Args> bool try_emplace(Args &&...args) {
    static_assert(std::is_constructible_v<T, Args...>,
                  "T must be constructible from Args");

    const auto write = producer_.write.load(std::memory_order_relaxed);
    const auto next = increment(write);
    if (next == producer_.cached_read) {
      producer_.cached_read =
          consumer_.read.load(std::memory_order_acquire);
      if (next == producer_.cached_read) {
        return false;
      }
    }

    Traits::construct(allocator_, storage_ + write,
                      std::forward<Args>(args)...);
    producer_.write.store(next, std::memory_order_release);
    return true;
  }

  template <class... Args> void emplace(Args &&...args) {
    while (!try_emplace(std::forward<Args>(args)...)) {
    }
  }

  bool try_push(const T &value) { return try_emplace(value); }
  bool try_push(T &&value) { return try_emplace(std::move(value)); }

  void push(const T &value) { emplace(value); }
  void push(T &&value) { emplace(std::move(value)); }

  [[nodiscard]] T *front() noexcept {
    const auto read = consumer_.read.load(std::memory_order_relaxed);
    if (read == consumer_.cached_write) {
      consumer_.cached_write =
          producer_.write.load(std::memory_order_acquire);
      if (read == consumer_.cached_write) {
        return nullptr;
      }
    }
    return storage_ + read;
  }

  [[nodiscard]] const T *front() const noexcept {
    return const_cast<Queue *>(this)->front();
  }

  void pop() noexcept {
    static_assert(std::is_nothrow_destructible_v<T>,
                  "T must have a noexcept destructor");
    const auto read = consumer_.read.load(std::memory_order_relaxed);
    assert(front() != nullptr && "pop requires a non-empty queue");
    Traits::destroy(allocator_, storage_ + read);
    consumer_.read.store(increment(read), std::memory_order_release);
  }

  bool try_pop(T &value) noexcept(std::is_nothrow_move_assignable_v<T>) {
    static_assert(std::is_move_assignable_v<T>, "T must be move assignable");
    T *item = front();
    if (item == nullptr) {
      return false;
    }
    value = std::move(*item);
    pop();
    return true;
  }

  [[nodiscard]] bool empty() const noexcept {
    return consumer_.read.load(std::memory_order_acquire) ==
           producer_.write.load(std::memory_order_acquire);
  }

  [[nodiscard]] std::size_t size() const noexcept {
    const auto read = consumer_.read.load(std::memory_order_acquire);
    const auto write = producer_.write.load(std::memory_order_acquire);
    return write >= read ? write - read : capacity_ - read + write;
  }

  [[nodiscard]] std::size_t capacity() const noexcept {
    return capacity_ - 1;
  }

private:
  [[nodiscard]] std::size_t increment(std::size_t index) const noexcept {
    ++index;
    return index == capacity_ ? 0 : index;
  }

  const std::size_t capacity_;
  [[no_unique_address]] Allocator allocator_;
  T *storage_;
  ProducerState producer_;
  ConsumerState consumer_;
};

} // namespace spsc
