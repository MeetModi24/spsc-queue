cmake_minimum_required(VERSION 3.16)
project(spsc_queue VERSION 1.0.0 LANGUAGES CXX)

add_library(spsc_queue INTERFACE)
add_library(spsc::queue ALIAS spsc_queue)
target_compile_features(spsc_queue INTERFACE cxx_std_17)
target_include_directories(spsc_queue INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)

option(SPSC_BUILD_TESTS "Build tests" ON)
option(SPSC_BUILD_BENCHMARKS "Build benchmarks" ON)
option(SPSC_BUILD_EXAMPLES "Build examples" ON)

find_package(Threads REQUIRED)

if(SPSC_BUILD_TESTS)
  enable_testing()
  add_executable(queue_test tests/queue_test.cpp)
  target_link_libraries(queue_test PRIVATE spsc::queue Threads::Threads)
  add_test(NAME queue_test COMMAND queue_test)
endif()

if(SPSC_BUILD_BENCHMARKS)
  add_executable(queue_benchmark benchmarks/queue_benchmark.cpp)
  target_link_libraries(queue_benchmark PRIVATE spsc::queue Threads::Threads)

  add_executable(latency_benchmark benchmarks/latency_benchmark.cpp)
  target_link_libraries(latency_benchmark PRIVATE spsc::queue Threads::Threads)
endif()

if(SPSC_BUILD_EXAMPLES)
  add_executable(queue_example examples/basic.cpp)
  target_link_libraries(queue_example PRIVATE spsc::queue Threads::Threads)

  add_executable(market_data_example examples/market_data_pipeline.cpp)
  target_link_libraries(market_data_example PRIVATE spsc::queue Threads::Threads)
endif()

include(GNUInstallDirs)
install(TARGETS spsc_queue EXPORT spsc_queue_targets)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT spsc_queue_targets
  FILE spsc_queue-config.cmake
  NAMESPACE spsc::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/spsc_queue)
