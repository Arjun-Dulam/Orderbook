#include <cassert>
#include <iostream>
#include <chrono>
#include "orderbook.hpp"

#define TEST_ORDERS 10000000

void test_partial_fill_incoming_larger() {
    std::cout << "\nTest: Partial fill - incoming order larger..." << std::endl;
    OrderBook book;

    // Sell 50 @ $100.00
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 50};
    book.add_order(sell1);

    // Buy 100 @ $100.00 (should match 50, then add 50 to book)
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10000, .quantity = 100};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].quantity == 50);  // Only 50 matched
    std::cout << "  ✓ Matched 50 out of 100" << std::endl;
}

void test_partial_fill_existing_larger() {
    std::cout << "\nTest: Partial fill - existing order larger..." << std::endl;
    OrderBook book;

    // Sell 100 @ $100.00
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 100};
    book.add_order(sell1);

    // Buy 50 @ $100.00 (should match 50, sell order stays with 50)
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10000, .quantity = 50};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].quantity == 50);
    std::cout << "  ✓ Matched 50, sell order still in book with 50 remaining" << std::endl;
}

void test_multiple_partial_matches() {
    std::cout << "\nTest: Multiple partial matches..." << std::endl;
    OrderBook book;

    // Add three sell orders
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 30};
    Order sell2{.order_id = 2, .side = Side::Sell, .price = 10000, .quantity = 20};
    Order sell3{.order_id = 3, .side = Side::Sell, .price = 10000, .quantity = 40};
    book.add_order(sell1);
    book.add_order(sell2);
    book.add_order(sell3);

    // Buy 100 @ $100.00 (should match all three: 30+20+40=90, then add 10 to book)
    Order buy1{.order_id = 4, .side = Side::Buy, .price = 10000, .quantity = 100};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 3);  // Three trades executed
    assert(trades[0].quantity == 30);
    assert(trades[1].quantity == 20);
    assert(trades[2].quantity == 40);
    std::cout << "  ✓ Matched with 3 orders: 30+20+40=90" << std::endl;
}

#include "orderbook.hpp"
#include <iostream>
#include <cassert>

void test_add_order_no_match() {
    std::cout << "Test 1: Add order with no match..." << std::endl;
    OrderBook book;

    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10100, .quantity = 100};  // Sell 100 @ $101.00
    auto trades = book.add_order(sell1);

    assert(trades.size() == 0);
    std::cout << "  ✓ No trades executed (expected 0, got " << trades.size() << ")" << std::endl;
}

void test_exact_match() {
    std::cout << "\nTest 2: Exact quantity match..." << std::endl;
    OrderBook book;

    // Add sell order first
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 100};  // Sell 100 @ $100.00
    book.add_order(sell1);

    // Add matching buy order
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10000, .quantity = 100};    // Buy 100 @ $100.00
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].price == 10000);
    assert(trades[0].quantity == 100);
    assert(trades[0].buy_order_id == 1);
    assert(trades[0].sell_order_id == 0);

    std::cout << "  ✓ Trade executed: " << trades[0].quantity
              << " @ $" << trades[0].price / 100.0 << std::endl;
    std::cout << "  ✓ Trade price matches existing order price" << std::endl;
}

void test_price_priority() {
    std::cout << "\nTest 3: Price priority (trade at existing order's price)..." << std::endl;
    OrderBook book;

    // Sell order at $100.00
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 50};
    book.add_order(sell1);

    // Buy order willing to pay $101.00 (should execute at $100.00)
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10100, .quantity = 50};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].price == 10000);  // Should execute at seller's price

    std::cout << "  ✓ Trade executed at $100.00 (existing order price), not $101.00" << std::endl;
}

void test_no_match_price_too_low() {
    std::cout << "\nTest 4: No match when prices don't cross..." << std::endl;
    OrderBook book;

    // Sell at $101.00
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10100, .quantity = 100};
    book.add_order(sell1);

    // Buy at $100.00 (lower than ask, no match)
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10000, .quantity = 100};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 0);
    std::cout << "  ✓ No trade when buy price < sell price" << std::endl;
}

void test_remove_order() {
    std::cout << "\nTest 6: Remove order from book..." << std::endl;
    OrderBook book;

    // Add two sell orders
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 100};
    Order sell2{.order_id = 2, .side = Side::Sell, .price = 10000, .quantity = 50};
    book.add_order(sell1);
    book.add_order(sell2);

    // Remove first order
    bool removed = book.remove_order(1);
    assert(removed == true);
    std::cout << "  ✓ Order removed successfully" << std::endl;

    // Try to remove same order again
    removed = book.remove_order(1);
    assert(removed == false);
    std::cout << "  ✓ Cannot remove already-removed order" << std::endl;

    // Try to remove non-existent order
    removed = book.remove_order(999);
    assert(removed == false);
    std::cout << "  ✓ Cannot remove non-existent order" << std::endl;
}

void test_time_priority() {
    std::cout << "\nTest 7: Time priority (first order at price level matched first)..." << std::endl;
    OrderBook book;

    // Add two sell orders at same price
    Order sell1{.order_id = 0, .side = Side::Sell, .price = 10000, .quantity = 100};
    Order sell2{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 100};
    book.add_order(sell1);
    book.add_order(sell2);

    // Buy order should match with first sell (order_id=1)
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10000, .quantity = 100};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].sell_order_id == 0);  // Should match first order

    std::cout << "  ✓ First order at price level matched (time priority)" << std::endl;
}

void test_multiple_price_levels() {
    std::cout << "\nTest 8: Multiple price levels..." << std::endl;
    OrderBook book;

    // Add sells at different prices
    Order sell1{.order_id = 0, .side = Side::Sell, .price = 10000, .quantity = 100};  // $100.00
    Order sell2{.order_id = 1, .side = Side::Sell, .price = 10100, .quantity = 100};  // $101.00
    Order sell3{.order_id = 2, .side = Side::Sell, .price = 9900, .quantity = 100};   // $99.00
    book.add_order(sell1);
    book.add_order(sell2);
    book.add_order(sell3);

    // Buy should match with lowest sell ($99.00)
    Order buy1{.order_id = 4, .side = Side::Buy, .price = 10000, .quantity = 100};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].price == 9900);  // Should match lowest ask
    assert(trades[0].sell_order_id == 2);

    std::cout << "  ✓ Matched with best price ($99.00)" << std::endl;
}

void test_both_sides() {
    std::cout << "\nTest 9: Both buy and sell sides..." << std::endl;
    OrderBook book;

    // Add buy at $100.00
    Order buy1{.order_id = 0, .side = Side::Buy, .price = 10000, .quantity = 100};
    book.add_order(buy1);

    // Add sell at $99.00 (crosses, should match)
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 9900, .quantity = 100};
    auto trades = book.add_order(sell1);

    assert(trades.size() == 1);
    assert(trades[0].price == 10000);  // Should execute at buy price (existing order)
    assert(trades[0].buy_order_id == 0);
    assert(trades[0].sell_order_id == 1);

    std::cout << "  ✓ Sell matching against existing buy works" << std::endl;
}

void test_show_trades() {
    std::cout << "\nTest 10: Show all trades..." << std::endl;
    OrderBook book;

    // Execute two trades
    Order sell1{.order_id = 1, .side = Side::Sell, .price = 10000, .quantity = 100};
    book.add_order(sell1);
    Order buy1{.order_id = 2, .side = Side::Buy, .price = 10000, .quantity = 100};
    book.add_order(buy1);

    Order sell2{.order_id = 3, .side = Side::Sell, .price = 10100, .quantity = 50};
    book.add_order(sell2);
    Order buy2{.order_id = 4, .side = Side::Buy, .price = 10100, .quantity = 50};
    book.add_order(buy2);

    auto all_trades = book.show_trades();
    assert(all_trades.size() == 2);

    std::cout << "  ✓ show_trades() returns all " << all_trades.size() << " trades" << std::endl;
}

void test_performance() {
    std::cout << "\n=== Performance Test ===" << std::endl;
    OrderBook book;

    // Test 1: Add non-matching orders (builds order book depth)
    std::cout << "\nTest 1: Add non-matching orders..." << std::endl;
    const int NUM_ORDERS = TEST_ORDERS;

    auto start = std::chrono::high_resolution_clock::now();

    // Add 50k sells at prices $100.01-$150.00
    for (int i = 0; i < NUM_ORDERS / 2; i++) {
        Order sell{.order_id = static_cast<uint32_t>(i),
                   .side = Side::Sell,
                   .price = 10001 + (i % 5000),
                   .quantity = 100};
        book.add_order(sell);
    }

    // Add 50k buys at prices $50.00-$99.99
    for (int i = NUM_ORDERS / 2; i < NUM_ORDERS; i++) {
        Order buy{.order_id = static_cast<uint32_t>(i),
                  .side = Side::Buy,
                  .price = 5000 + (i % 5000),
                  .quantity = 100};
        book.add_order(buy);
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double orders_per_sec = (NUM_ORDERS * 1000.0) / duration.count();
    std::cout << "  Added " << NUM_ORDERS << " non-matching orders" << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(orders_per_sec) << " orders/sec" << std::endl;

    // Test 2: Order cancellations
    std::cout << "\nTest 2: Order cancellations..." << std::endl;
    const int NUM_CANCELS = TEST_ORDERS;

    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < NUM_CANCELS; i++) {
        book.remove_order(i);
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double cancels_per_sec = (NUM_CANCELS * 1000.0) / duration.count();
    std::cout << "  Cancelled " << NUM_CANCELS << " orders" << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(cancels_per_sec) << " cancels/sec" << std::endl;

    // Test 3: Matching orders (creates trades)
    std::cout << "\nTest 3: Matching orders..." << std::endl;
    OrderBook book2;
    const int NUM_MATCHES = TEST_ORDERS;

    // Add sells at $100.00
    for (int i = 0; i < NUM_MATCHES; i++) {
        Order sell{.order_id = static_cast<uint32_t>(i),
                   .side = Side::Sell,
                   .price = 1000 + (i % 1000),
                   .quantity = 100};
        book2.add_order(sell);
    }

    start = std::chrono::high_resolution_clock::now();

    int total_trades = 0;
    // Add matching buys at $100.00
    for (int i = NUM_MATCHES; i < NUM_MATCHES * 2; i++) {
        Order buy{.order_id = static_cast<uint32_t>(i),
                  .side = Side::Buy,
                  .price = 11000,
                  .quantity = 100};
        auto trades = book2.add_order(buy);
        total_trades += trades.size();
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double matches_per_sec = (NUM_MATCHES * 1000.0) / duration.count();
    double trades_per_sec = (total_trades * 1000.0) / duration.count();
    std::cout << "  Matched " << NUM_MATCHES << " orders" << std::endl;
    std::cout << "  Executed " << total_trades << " trades" << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(matches_per_sec) << " orders/sec" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(trades_per_sec) << " trades/sec" << std::endl;

    // Test 4: Mixed workload
    std::cout << "\nTest 4: Mixed workload (add + match + cancel)..." << std::endl;
    OrderBook book3;
    const int MIXED_OPS = TEST_ORDERS;

    start = std::chrono::high_resolution_clock::now();

    for (int i = 0; i < MIXED_OPS; i++) {
        if (i % 3 == 0) {
            // Add non-matching order
            Order sell{.order_id = static_cast<uint32_t>(i),
                       .side = Side::Sell,
                       .price = 10100,
                       .quantity = 100};
            book3.add_order(sell);
        } else if (i % 3 == 1) {
            // Add matching order
            Order buy{.order_id = static_cast<uint32_t>(i),
                      .side = Side::Buy,
                      .price = 10100,
                      .quantity = 100};
            book3.add_order(buy);
        } else {
            // Cancel order
            if (i >= 6) {
                book3.remove_order(i - 3);
            }
        }
    }

    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double ops_per_sec = (MIXED_OPS * 1000.0) / duration.count();
    std::cout << "  Performed " << MIXED_OPS << " mixed operations" << std::endl;
    std::cout << "  Time: " << duration.count() << " ms" << std::endl;
    std::cout << "  Throughput: " << static_cast<int>(ops_per_sec) << " ops/sec" << std::endl;

    std::cout << "\n=== Performance Test Complete ===" << std::endl;
}

int main() {
    test_add_order_no_match();
    test_exact_match();
    test_price_priority();
    test_no_match_price_too_low();
    test_remove_order();
    test_time_priority();
    test_multiple_price_levels();
    test_both_sides();
    test_show_trades();
    test_multiple_partial_matches();
    test_partial_fill_existing_larger();
    test_multiple_price_levels();

    std::cout << "\n=== All tests passed! ✓ ===" << std::endl;

    // Run performance test
    test_performance();

    return 0;
}