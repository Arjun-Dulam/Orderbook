#include <benchmark/benchmark.h>
#include <chrono>

#include "order_generator.hpp"
#include "../include/orderbook.hpp"
#include "../include/order.hpp"

static void BM_AddOrder_No_Match(benchmark::State &state) {
    OrderBook order_book;
    OrderGenerator order_gen(MarketConfig{});
    const size_t num_orders = 15000000;

    std::vector<Order> orders_to_add;
    orders_to_add.reserve(num_orders);

    for (size_t i = 0; i < num_orders; i++) {
        orders_to_add.push_back(order_gen.generate_order());
    } // order buffer to hold orders to be added to prevent order generation from being tested

    for (int i = 0; i < state.range(0); i++) {
        Order new_order = order_gen.generate_order();
        order_book.add_order(new_order);
    } // prepopulate orderbook for cold start and various hot starts
    // doesn't use any pregenerated orders in orders_to_add to maximize cache miss rate for accurate testing

    size_t order_idx = 0;

    for (auto _ : state) {
        auto trade = order_book.add_order(orders_to_add[order_idx]);
        benchmark::DoNotOptimize(trade);
        order_idx = (order_idx + 1) % num_orders;
    }

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_AddOrder_No_Match)
    -> Arg(0) // cold start
    -> Arg(1000)
    -> Arg(10000)
    -> Arg(100000)
    -> Arg(1000000);


static void BM_AddOrder_Latency(benchmark::State &state) {
    OrderBook order_book;
    OrderGenerator order_gen(MarketConfig{});
    const size_t num_orders = 15000000;

    std::vector<double> latencies;
    latencies.reserve(num_orders);
    std::vector<Order> orders_to_add;
    orders_to_add.reserve(num_orders);

    for (size_t i = 0; i < num_orders; i++) {
        orders_to_add.push_back(order_gen.generate_order());
    }

    for (int i = 0; i < state.range(0); i++) {
        Order new_order = order_gen.generate_order();
        order_book.add_order(new_order);
    }

    size_t order_idx = 0;

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        auto trade = order_book.add_order(orders_to_add[order_idx]);
        auto end = std::chrono::high_resolution_clock::now();

        double dur = std::chrono::duration<double, std::nano>(end - start).count();
        latencies.push_back(dur);
        benchmark::DoNotOptimize(trade);
        order_idx = (order_idx + 1) % num_orders;
    }

    std::sort(latencies.begin(), latencies.end());
    state.counters["p999"] = latencies[latencies.size() * .999];
    state.counters["p99"] = latencies[latencies.size() * .99];
    state.counters["p50"] = latencies[latencies.size() * .50];

    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_AddOrder_Latency)
    -> Arg(0)
    -> Arg(1000)
    -> Arg(10000)
    -> Arg(100000)
    -> Arg(1000000);

BENCHMARK_MAIN();