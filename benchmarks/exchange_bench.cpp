#include <benchmark/benchmark.h>
#include <thread>
#include <vector>
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

static void BM_Exchange_RealisticWorkload(benchmark::State& state) {
    const int num_threads = state.range(0);
    const int num_symbols = 10;

    Exchange exchange;
    std::vector<std::string> symbols = {
        "AAPL", "GOOG", "MSFT", "AMZN", "TSLA",
        "NVDA", "META", "NFLX", "AMD", "INTC"
    };

    for (const auto& symbol : symbols) {
        exchange.add_symbol(symbol);
    }

    for (auto _ : state) {
        std::vector<std::thread> threads;

        for (int i = 0; i < num_threads; i++) {
            threads.emplace_back([&exchange, &symbols, num_symbols, i]() {
                MarketConfig config;
                config.base_price = 10000;
                OrderGenerator gen(config, 42 + i);

                for (int j = 0; j < 500; j++) {
                    Order order = gen.generate_order();
                    // Each thread picks symbols in round-robin
                    std::string symbol = symbols[(i + j) % num_symbols];
                    exchange.add_order(symbol, order);
                }
            });
        }

        for (auto& t : threads) {
            t.join();
        }
    }

    state.SetItemsProcessed(state.iterations() * num_threads * 500);
}
BENCHMARK(BM_Exchange_RealisticWorkload)
    ->Arg(1)
    ->Arg(2)
    ->Arg(4)
    ->Arg(8)
    ->UseRealTime();

BENCHMARK_MAIN();
