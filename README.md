# SPSC Queue

A compact, bounded single-producer/single-consumer queue for predictable
inter-thread communication in latency-sensitive C++ applications.

## Design

- Fixed-capacity ring buffer with no allocation after construction
- Acquire/release synchronization between one producer and one consumer
- Producer and consumer state isolated on separate cache lines
- Locally cached remote indices to limit cache-coherency traffic
- In-place construction and support for move-only values
- Arbitrary capacities; powers of two are not required

The queue is safe only when one thread produces and one thread consumes.
`try_push` and `try_emplace` return `false` when the queue is full. `front`
returns `nullptr` when it is empty.

## Example

```cpp
#include <spsc/queue.hpp>

spsc::Queue<int> queue(1024);

if (queue.try_push(42)) {
  // published to the consumer
}

if (int *value = queue.front()) {
  process(*value);
  queue.pop();
}
```

## Build and test

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
ctest --test-dir build --output-on-failure
./build/queue_benchmark
```

Benchmark results depend on CPU topology, affinity, compiler, and system load.
Measure on the target machine before quoting performance numbers.

## Requirements

- C++17 compiler
- CMake 3.16 or newer for the included build

## License

MIT
