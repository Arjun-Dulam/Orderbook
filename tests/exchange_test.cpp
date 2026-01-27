#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include "../include/exchange.hpp"

TEST(ExchangeTest, SingleThreadedCorrectness) {
    Exchange exchange;
    exchange.add_symbol("AAPL");

    Order buy_order(10000, 100, Side::Buy, false);
    Order sell_order(10000, 100, Side::Sell, false);

    std::string symbol = "AAPL";
    auto trades1 = exchange.add_order(symbol, buy_order);
    auto trades2 = exchange.add_order(symbol, sell_order);

    ASSERT_EQ(trades1.size(), 0);
    ASSERT_EQ(trades2.size(), 1);
    EXPECT_EQ(trades2[0].price, 10000);
    EXPECT_EQ(trades2[0].quantity, 100);
}

TEST(ExchangeTest, MultiThreadedStressTest) {
    Exchange exchange;
    std::vector<std::string> symbols = {"AAPL", "GOOG", "MSFT", "TSLA"};

    for (const auto& symbol : symbols) {
        exchange.add_symbol(symbol);
    }

    std::vector<std::thread> threads;

    for (int i = 0; i < 4; i++) {
        threads.emplace_back([&exchange, &symbols, i]() {
            std::string symbol = symbols[i];

            for (int j = 0; j < 1000; j++) {
                Order order(10000 + (j % 100), 10,
                           (j % 2 == 0) ? Side::Buy : Side::Sell, false);
                exchange.add_order(symbol, order);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    SUCCEED();
}
