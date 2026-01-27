cmake_minimum_required(VERSION 3.21)
project(StockExchange VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

include(FetchContent)

FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.3
)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)

set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(benchmark googletest)

find_package(Threads REQUIRED)

set(ORDERBOOK_SOURCES
    src/orderbook.cpp
    src/order.cpp
)

set(EXCHANGE_SOURCES
    src/exchange.cpp
    src/orderbook.cpp
    src/order.cpp
)

add_executable(OrderBookBenchmark
    benchmarks/orderbook_bench.cpp
    benchmarks/order_generator.cpp
    ${ORDERBOOK_SOURCES}
)
target_link_libraries(OrderBookBenchmark benchmark::benchmark)
target_compile_options(OrderBookBenchmark PRIVATE -O3 -march=native -DNDEBUG -flto)
target_link_options(OrderBookBenchmark PRIVATE -flto)

add_executable(ExchangeBenchmark
    benchmarks/exchange_bench.cpp
    benchmarks/order_generator.cpp
    ${EXCHANGE_SOURCES}
)
target_link_libraries(ExchangeBenchmark benchmark::benchmark Threads::Threads)
target_compile_options(ExchangeBenchmark PRIVATE -O3 -march=native -DNDEBUG)

add_executable(OrderBookTests
    tests/test.cpp
    ${ORDERBOOK_SOURCES}
)
target_link_libraries(OrderBookTests GTest::gtest_main)
target_include_directories(OrderBookTests PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_options(OrderBookTests PRIVATE -g -O0 -fsanitize=address -fsanitize=undefined)
target_link_options(OrderBookTests PRIVATE -fsanitize=address -fsanitize=undefined)

add_executable(ExchangeTests
    tests/exchange_test.cpp
    ${EXCHANGE_SOURCES}
)
target_link_libraries(ExchangeTests GTest::gtest_main Threads::Threads)
target_compile_options(ExchangeTests PRIVATE -g -O0 -fsanitize=thread)
target_link_options(ExchangeTests PRIVATE -fsanitize=thread)

enable_testing()
add_test(NAME OrderBookTests COMMAND OrderBookTests)
add_test(NAME ExchangeTests COMMAND ExchangeTests)