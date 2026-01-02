#include <cassert>
#include <iostream>
#include "orderbook.hpp"

void test_partial_fill_incoming_larger() {
    std::cout << "\nTest: Partial fill - incoming order larger..." << std::endl;
    OrderBook book;

    // Sell 50 @ $100.00
    Order sell1{1, Side::Sell, 10000, 50, 0};
    book.add_order(sell1);

    // Buy 100 @ $100.00 (should match 50, then add 50 to book)
    Order buy1{2, Side::Buy, 10000, 100, 0};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].quantity == 50);  // Only 50 matched
    std::cout << "  ✓ Matched 50 out of 100" << std::endl;
}

void test_partial_fill_existing_larger() {
    std::cout << "\nTest: Partial fill - existing order larger..." << std::endl;
    OrderBook book;

    // Sell 100 @ $100.00
    Order sell1{1, Side::Sell, 10000, 100, 0};
    book.add_order(sell1);

    // Buy 50 @ $100.00 (should match 50, sell order stays with 50)
    Order buy1{2, Side::Buy, 10000, 50, 0};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].quantity == 50);
    std::cout << "  ✓ Matched 50, sell order still in book with 50 remaining" << std::endl;
}

void test_multiple_partial_matches() {
    std::cout << "\nTest: Multiple partial matches..." << std::endl;
    OrderBook book;

    // Add three sell orders
    Order sell1{1, Side::Sell, 10000, 30, 0};
    Order sell2{2, Side::Sell, 10000, 20, 0};
    Order sell3{3, Side::Sell, 10000, 40, 0};
    book.add_order(sell1);
    book.add_order(sell2);
    book.add_order(sell3);

    // Buy 100 @ $100.00 (should match all three: 30+20+40=90, then add 10 to book)
    Order buy1{4, Side::Buy, 10000, 100, 0};
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

    Order sell1{1, Side::Sell, 10100, 100, 0};  // Sell 100 @ $101.00
    auto trades = book.add_order(sell1);

    assert(trades.size() == 0);
    std::cout << "  ✓ No trades executed (expected 0, got " << trades.size() << ")" << std::endl;
}

void test_exact_match() {
    std::cout << "\nTest 2: Exact quantity match..." << std::endl;
    OrderBook book;

    // Add sell order first
    Order sell1{1, Side::Sell, 10000, 100, 0};  // Sell 100 @ $100.00
    book.add_order(sell1);

    // Add matching buy order
    Order buy1{2, Side::Buy, 10000, 100, 0};    // Buy 100 @ $100.00
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].price == 10000);
    assert(trades[0].quantity == 100);
    assert(trades[0].buy_order_id == 2);
    assert(trades[0].sell_order_id == 1);

    std::cout << "  ✓ Trade executed: " << trades[0].quantity
              << " @ $" << trades[0].price / 100.0 << std::endl;
    std::cout << "  ✓ Trade price matches existing order price" << std::endl;
}

void test_price_priority() {
    std::cout << "\nTest 3: Price priority (trade at existing order's price)..." << std::endl;
    OrderBook book;

    // Sell order at $100.00
    Order sell1{1, Side::Sell, 10000, 50, 0};
    book.add_order(sell1);

    // Buy order willing to pay $101.00 (should execute at $100.00)
    Order buy1{2, Side::Buy, 10100, 50, 0};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].price == 10000);  // Should execute at seller's price

    std::cout << "  ✓ Trade executed at $100.00 (existing order price), not $101.00" << std::endl;
}

void test_no_match_price_too_low() {
    std::cout << "\nTest 4: No match when prices don't cross..." << std::endl;
    OrderBook book;

    // Sell at $101.00
    Order sell1{1, Side::Sell, 10100, 100, 0};
    book.add_order(sell1);

    // Buy at $100.00 (lower than ask, no match)
    Order buy1{2, Side::Buy, 10000, 100, 0};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 0);
    std::cout << "  ✓ No trade when buy price < sell price" << std::endl;
}

void test_remove_order() {
    std::cout << "\nTest 6: Remove order from book..." << std::endl;
    OrderBook book;

    // Add two sell orders
    Order sell1{1, Side::Sell, 10000, 100, 0};
    Order sell2{2, Side::Sell, 10000, 50, 0};
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
    Order sell1{1, Side::Sell, 10000, 100, 0};
    Order sell2{2, Side::Sell, 10000, 100, 0};
    book.add_order(sell1);
    book.add_order(sell2);

    // Buy order should match with first sell (order_id=1)
    Order buy1{3, Side::Buy, 10000, 100, 0};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].sell_order_id == 1);  // Should match first order

    std::cout << "  ✓ First order at price level matched (time priority)" << std::endl;
}

void test_multiple_price_levels() {
    std::cout << "\nTest 8: Multiple price levels..." << std::endl;
    OrderBook book;

    // Add sells at different prices
    Order sell1{1, Side::Sell, 10000, 100, 0};  // $100.00
    Order sell2{2, Side::Sell, 10100, 100, 0};  // $101.00
    Order sell3{3, Side::Sell, 9900, 100, 0};   // $99.00
    book.add_order(sell1);
    book.add_order(sell2);
    book.add_order(sell3);

    // Buy should match with lowest sell ($99.00)
    Order buy1{4, Side::Buy, 10000, 100, 0};
    auto trades = book.add_order(buy1);

    assert(trades.size() == 1);
    assert(trades[0].price == 9900);  // Should match lowest ask
    assert(trades[0].sell_order_id == 3);

    std::cout << "  ✓ Matched with best price ($99.00)" << std::endl;
}

void test_both_sides() {
    std::cout << "\nTest 9: Both buy and sell sides..." << std::endl;
    OrderBook book;

    // Add buy at $100.00
    Order buy1{1, Side::Buy, 10000, 100, 0};
    book.add_order(buy1);

    // Add sell at $99.00 (crosses, should match)
    Order sell1{2, Side::Sell, 9900, 100, 0};
    auto trades = book.add_order(sell1);

    assert(trades.size() == 1);
    assert(trades[0].price == 10000);  // Should execute at buy price (existing order)
    assert(trades[0].buy_order_id == 1);
    assert(trades[0].sell_order_id == 2);

    std::cout << "  ✓ Sell matching against existing buy works" << std::endl;
}

void test_show_trades() {
    std::cout << "\nTest 10: Show all trades..." << std::endl;
    OrderBook book;

    // Execute two trades
    Order sell1{1, Side::Sell, 10000, 100, 0};
    book.add_order(sell1);
    Order buy1{2, Side::Buy, 10000, 100, 0};
    book.add_order(buy1);

    Order sell2{3, Side::Sell, 10100, 50, 0};
    book.add_order(sell2);
    Order buy2{4, Side::Buy, 10100, 50, 0};
    book.add_order(buy2);

    auto all_trades = book.show_trades();
    assert(all_trades.size() == 2);

    std::cout << "  ✓ show_trades() returns all " << all_trades.size() << " trades" << std::endl;
}

int main() {
    std::cout << "=== OrderBook Day 4 Tests ===" << std::endl;

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
    std::cout << "\nDay 4 Complete! Ready for Day 5 (partial fills)." << std::endl;

    return 0;
}