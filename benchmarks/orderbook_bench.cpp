#include <benchmark/benchmark.h>
#include <chrono>

#include "order_generator.hpp"
#include "../include/orderbook.hpp"
#include "../include/order.hpp"

#define NUM_ORDERS 15000000

static void BM_AddOrder_No_Match(benchmark::State &state) {
    OrderBook order_book;
    OrderGenerator order_gen(MarketConfig{});
    const size_t num_orders = NUM_ORDERS;

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
    const size_t num_orders = NUM_ORDERS;

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


static void BM_RemoveOrder_VaryDepth(benchmark::State &state) {
    OrderBook order_book;
    MarketConfig cfg;
    OrderGenerator order_generator(cfg);

    std::vector<uint32_t> orders_to_remove;
    orders_to_remove.reserve(state.range(0) / 3 + 1);

    for (int i = 0; i < state.range(0); i++) {
        Order new_order = order_generator.generate_order();
        order_book.add_order(new_order);

        if (i % 3 == 0) {
            orders_to_remove.push_back(new_order.get_order_id());
        }
    }

    OrderBook order_book_backup = order_book;

    size_t orderIdx = 0;

    for (auto _ : state) {
        if (orderIdx >= orders_to_remove.size() - 1) {
            state.PauseTiming();
            orderIdx = 0;
            order_book = order_book_backup;
            state.ResumeTiming();
        }
        order_book.remove_order(orders_to_remove[orderIdx]);

        orderIdx++;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_RemoveOrder_VaryDepth)
    -> Arg(10000)
    -> Arg(100000)
    -> Arg(1000000)
    -> Arg(10000000)
    -> Arg(NUM_ORDERS);

/**
static void BM_MixedWorkload(benchmark::State &state) {
    OrderBook order_book;
    OrderGenerator order_gen(MarketConfig{});
    const size_t num_orders = NUM_ORDERS;

    std::vector<Order> orders_to_add;
    orders_to_add.reserve(num_orders);

    for (size_t i = 0; i < num_orders; i++) {
        orders_to_add.push_back(order_gen.generate_order());
    }

    for (int i = 0; i < state.range(0); i++) {
        Order new_order = order_gen.generate_order();
        order_book.add_order(new_order);
    }

    for (auto _ : state) {

    }

}

**/

BENCHMARK_MAIN();