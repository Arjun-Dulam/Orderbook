cmake_minimum_required(VERSION 3.21)

project(StockExchange VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# set(CMAKE_CXX_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(SOURCE
    tests/test.cpp
    src/orderbook.cpp
    src/order.cpp
    benchmarks/order_generator.cpp
    benchmarks/orderbook_bench.cpp
)

include(FetchContent)

FetchContent_Declare(
        benchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG v1.8.3
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark)

set(BENCH_SOURCE
    benchmarks/orderbook_bench.cpp
    benchmarks/order_generator.cpp
    src/orderbook.cpp
    src/order.cpp
)

add_executable(OrderBookBenchmark ${BENCH_SOURCE})
target_link_libraries(OrderBookBenchmark benchmark::benchmark)

add_executable(StockExchange ${SOURCE})
