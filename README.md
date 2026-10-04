# SPSC Queue

A compact, bounded single-producer/single-consumer queue for predictable
inter-thread communication in latency-sensitive C++ applications.

**Main implementation:** [`include/spsc/queue.hpp`](include/spsc/queue.hpp)

## Design

- Fixed-capacity ring buffer with no allocation after construction
- Acquire/release synchronization between one producer and one consumer
- Producer and consumer state isolated on separate cache lines
- Locally cached remote indices to limit cache-coherency traffic
- In-place construction and support for move-only values
- Blocking and non-blocking producer APIs plus a non-blocking consumer helper
- Arbitrary capacities; powers of two are not required

The queue is safe only when one thread produces and one thread consumes.
`try_push` and `try_emplace` return `false` when the queue is full. `front`
returns `nullptr` when it is empty.

## Repository layout

```text
include/spsc/queue.hpp              queue implementation
tests/queue_test.cpp                API, lifetime, and concurrency tests
benchmarks/queue_benchmark.cpp      producer/consumer throughput
benchmarks/latency_benchmark.cpp    two-thread round-trip latency
examples/basic.cpp                  minimal usage example
examples/market_data_pipeline.cpp   market-data handoff example
```

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
./build/latency_benchmark
./build/market_data_example
```

Benchmark results depend on CPU topology, affinity, compiler, and system load.
Measure on the target machine before quoting performance numbers.

For stable measurements, use a release build, isolate the benchmark cores,
pin each thread to a physical core, and disable frequency scaling where the
test environment permits it.

## Requirements

- C++17 compiler
- CMake 3.16 or newer for the included build

## License

MIT
