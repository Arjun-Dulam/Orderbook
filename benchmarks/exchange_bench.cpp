#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
#include <random>
#include "../include/exchange.hpp"
#include "order_generator.hpp"

static void BM_Exchange_SingleThreadSingleSymbol(benchmark::State& state) {
    Exchange exchange;
    exchange.add_symbol("AAPL");

    MarketConfig config;
    config.base_price = 10000;
    config.price_std_dev = 50;
    OrderGenerator gen(config, 42);

    for (auto _ : state) {
        Order order = gen.generate_order();
        std::string symbol = "AAPL";
        exchange.add_order(symbol, order);
    }

    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Exchange_SingleThreadSingleSymbol);

static void BM_Exchange_MultiThreadMultiSymbol(benchmark::State& state) {
    const int num_threads = state.range(0);

    Exchange exchange;
    std::vector<std::string> symbols;
    for (int i = 0; i < num_threads; i++) {
        std::string symbol = "SYM" + std::to_string(i);
        exchange.add_symbol(symbol);
        symbols.push_back(symbol);
    }

    for (auto _ : state) {
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back([&exchange, &symbols, i]() {
                MarketConfig config;
                config.base_price = 10000;
                OrderGenerator gen(config, 42 + i);

                for (int j = 0; j < 1000; j++) {
                    Order order = gen.generate_order();
                    std::string symbol = symbols[i];
                    exchange.add_order(symbol, order);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 1000);
}
BENCHMARK(BM_Exchange_MultiThreadMultiSymbol)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->UseRealTime();

static void BM_Exchange_MultiThreadSameSymbol(benchmark::State& state) {
    const int num_threads = state.range(0);

    Exchange exchange;
    exchange.add_symbol("AAPL");

    for (auto _ : state) {
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back([&exchange, i]() {
                MarketConfig config;
                config.base_price = 10000;
                OrderGenerator gen(config, 42 + i);

                for (int j = 0; j < 1000; j++) {
                    Order order = gen.generate_order();
                    std::string symbol = "AAPL";
                    exchange.add_order(symbol, order);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 1000);
}
BENCHMARK(BM_Exchange_MultiThreadSameSymbol)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->UseRealTime();

// 100 symbols, fixed thread pool, random distribution (realistic)
static void BM_Exchange_RealisticWorkload(benchmark::State& state) {
    const int num_threads = state.range(0);
    const int num_symbols = 100;

    Exchange exchange;
    std::vector<std::string> symbols;
    symbols.reserve(num_symbols);

    for (int i = 0; i < num_symbols; i++) {
        std::string symbol = "SYM" + std::to_string(i);
        exchange.add_symbol(symbol);
        symbols.push_back(symbol);
    }

    for (auto _ : state) {
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back([&exchange, &symbols, num_symbols, i]() {
                MarketConfig config;
                config.base_price = 10000;
                OrderGenerator gen(config, 42 + i);

                std::mt19937 rng(42 + i);
                std::uniform_int_distribution<int> symbol_dist(0, num_symbols - 1);

                for (int j = 0; j < 1000; j++) {
                    Order order = gen.generate_order();
                    // Random symbol - reduces collision probability
                    std::string symbol = symbols[symbol_dist(rng)];
                    exchange.add_order(symbol, order);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 1000);
}
BENCHMARK(BM_Exchange_RealisticWorkload)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->UseRealTime();

BENCHMARK_MAIN();
