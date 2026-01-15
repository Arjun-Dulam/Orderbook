#include <benchmark/benchmark.h>
#include <chrono>
#include <iostream>

#include "order_generator.hpp"
#include "../include/orderbook.hpp"
#include "../include/order.hpp"

static void BM_AddOrder_No_Match(benchmark::State &state) {
    OrderBook order_book;
    OrderGenerator order_gen(MarketConfig{});
    const size_t num_orders = 15000000;

    std::vector<Order> orders_to_add;
    orders_to_add.reserve(num_orders);

    std::vector<size_t> order_indices;
    order_indices.reserve(num_orders);

    for (size_t i = 0; i < num_orders; i++) {
        Order new_order = order_gen.generate_order();
        if (new_order.side == Side::Buy) {new_order.price -= 500;}
        else {new_order.price += 500;}
        orders_to_add.push_back(new_order);
    }

    for (int i = 0; i < state.range(0); i++) {
        Order new_order = order_gen.generate_order();
        if (new_order.side == Side::Buy) {new_order.price -= 500;}
        else {new_order.price += 500;}
        order_book.add_order(new_order);
    }

    for (size_t i = 0; i < num_orders; i++) {
        order_indices.push_back(i);
    }

    std::mt19937 rng(67);
    shuffle(order_indices.begin(), order_indices.end(), rng);

    size_t order_idx = 0;

    for (auto _ : state) {
        auto trade = order_book.add_order(orders_to_add[order_indices[order_idx]]);

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
-> Arg(1000000)
-> Arg(15000000);

static void BM_AddOrder_Latency(benchmark::State &state) {
    OrderBook order_book;
    OrderGenerator order_gen(MarketConfig{});
    const size_t num_orders = 15000000;

    std::vector<double> latencies;
    latencies.reserve(num_orders);
    std::vector<Order> orders_to_add;
    orders_to_add.reserve(num_orders);
    std::vector<size_t> order_indices;
    order_indices.reserve(num_orders);

    for (size_t i = 0; i < num_orders; i++) {
        Order new_order = order_gen.generate_order();
        if (new_order.side == Side::Buy) {new_order.price -= 500;}
        else {new_order.price += 500;}
        orders_to_add.push_back(new_order);
    }

    for (int i = 0; i < state.range(0); i++) {
        Order new_order = order_gen.generate_order();
        if (new_order.side == Side::Buy) {new_order.price -= 500;}
        else {new_order.price += 500;}
        order_book.add_order(new_order);
    }

    size_t order_idx = 0;

    for (size_t i = 0; i < num_orders; i++) {
        order_indices.push_back(i);
    }

    std::mt19937 rng(67);
    shuffle(order_indices.begin(), order_indices.end(), rng);

    for (auto _ : state) {
        auto start = std::chrono::high_resolution_clock::now();
        auto trade = order_book.add_order(orders_to_add[order_indices[order_idx]]);
        auto end = std::chrono::high_resolution_clock::now();

        double dur = std::chrono::duration<double, std::nano>(end - start).count();
        latencies.push_back(dur);
        benchmark::DoNotOptimize(trade);
        order_idx = (order_idx + 1) % num_orders;
    }

    std::sort(latencies.begin(), latencies.end());
    state.counters["p999"] = latencies[static_cast<size_t>(latencies.size() * 0.999)];
    state.counters["p99"] = latencies[static_cast<size_t>(latencies.size() * 0.99)];
    state.counters["p50"] = latencies[static_cast<size_t>(latencies.size() * 0.50)];


    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_AddOrder_Latency)
-> Arg(0)
-> Arg(1000)
-> Arg(10000)
-> Arg(100000)
-> Arg(1000000)
-> Arg(15000000);


static void BM_RemoveOrder_VaryDepth(benchmark::State &state) {
    OrderBook order_book;
    MarketConfig cfg;
    OrderGenerator order_gen(cfg);
    const size_t depth = state.range(0);

    std::vector<uint32_t> orders_to_remove;
    orders_to_remove.reserve(depth);

    for (size_t i = 0; i < depth; i++) {
        Order new_order = order_gen.generate_order();
        if (new_order.side == Side::Buy) { new_order.price -= 500; }
        else { new_order.price += 500; }
        order_book.add_order(new_order);
        orders_to_remove.push_back(new_order.get_order_id());
    }

    std::mt19937 rng(67);
    std::shuffle(orders_to_remove.begin(), orders_to_remove.end(), rng);

    const size_t half = depth / 2;

    for (auto _ : state) {
        for (size_t i = 0; i < half; i++) {
            auto result = order_book.remove_order(orders_to_remove[i]);
            benchmark::DoNotOptimize(result);
        }
    }

    state.SetItemsProcessed(half);
}

BENCHMARK(BM_RemoveOrder_VaryDepth)
-> Arg(1000) -> Iterations(1)
-> Arg(10000) -> Iterations(1)
-> Arg(100000) -> Iterations(1)
-> Arg(1000000) -> Iterations(1)
-> Arg(15000000) -> Iterations(1);

static void BM_MixedWorkload(benchmark::State &state) {
    OrderBook order_book;
    MarketConfig cfg;
    OrderGenerator order_gen(cfg);
    const size_t num_orders = 10000000;

    std::vector<Order> orders_to_add;
    orders_to_add.reserve(num_orders);

    std::vector<uint32_t> orders_to_remove;
    orders_to_remove.reserve(num_orders);

    std::vector<bool> should_cancel_vec;
    should_cancel_vec.reserve(num_orders);

    for (size_t i = 0; i < num_orders; i++) { // Order Stream
        Order new_order = order_gen.generate_order();

        if (new_order.side == Side::Buy) {new_order.price -= 100;}
        else {new_order.price += 100;}

        orders_to_add.push_back(new_order);
    }

    for (size_t i = 0; i < state.range(0); i++) { // Prepopulating Orderbook
        Order new_order = order_gen.generate_order();

        if (new_order.side == Side::Buy) {new_order.price -= 100;}
        else {new_order.price += 100;}

        order_book.add_order(new_order);
    }

    for (size_t i = 0; i < num_orders; i++) {
        should_cancel_vec.push_back(order_gen.should_cancel());
    }

    OrderBook backup_order_book = order_book;
    size_t orderIdx = 0;

    for (auto _ : state) {
        if (orderIdx == num_orders - 1) {
            state.PauseTiming();
            orderIdx = 0;
            order_book = backup_order_book;
            state.ResumeTiming();
        }

        Order &new_order = orders_to_add[orderIdx];
        order_book.add_order(new_order);

        if (new_order.quantity > 0) {
            orders_to_remove.push_back(new_order.get_order_id());
        }

        if (should_cancel_vec[orderIdx] && !orders_to_remove.empty()) {
            size_t removal_index =  orders_to_remove.size() > 100 ? orders_to_remove.size() - 100 : 0;
            order_book.remove_order(orders_to_remove[removal_index]);
        }
        orderIdx++;
    }
    state.SetItemsProcessed(state.iterations());
}

BENCHMARK(BM_MixedWorkload)
-> Arg(0)
-> Arg(1000)
-> Arg(10000)
-> Arg(100000)
-> Arg(1000000)
-> Arg(15000000);

BENCHMARK_MAIN();