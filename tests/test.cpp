#include <iostream>
#include "orderbook.hpp"

int main() {
    OrderBook book;

    // Create some orders
    Order buy1{1, Buy, 10000, 100, 0};   // Buy 100 @ $100.00
    Order buy2{2, Buy, 10000, 50, 0};    // Buy 50 @ $100.00 (same price)
    Order buy3{3, Buy, 9900, 75, 0};     // Buy 75 @ $99.00 (lower price)

    Order sell1{4, Sell, 10100, 60, 0};  // Sell 60 @ $101.00
    Order sell2{5, Sell, 10200, 80, 0};  // Sell 80 @ $102.00

    // Add them
    std::cout << "Adding orders..." << std::endl;
    auto trades1 = book.add_order(buy1);
    auto trades2 = book.add_order(buy2);
    auto trades3 = book.add_order(buy3);
    auto trades4 = book.add_order(sell1);
    auto trades5 = book.add_order(sell2);

    std::cout << "All orders added successfully!" << std::endl;
    std::cout << "Total trades: " << book.show_trades().size() << std::endl;

    return 0;
}