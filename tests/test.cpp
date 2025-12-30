#include <iostream>
#include "order.hpp"
#include "orderbook.hpp"

int main() {
    OrderBook book;

    Order buy_order{1, Buy, 10050, 100, 0};
    Order sell_order{2, Sell, 10100, 50, 0};

    auto trades = book.add_order(buy_order);
    std::cout << "Trades from buy order: " << trades.size() << std::endl;

    trades = book.add_order(sell_order);
    std::cout << "Trades from sell order: " << trades.size() << std::endl;

    auto all_trades = book.show_trades();
    std::cout << "Total trades: " << all_trades.size() << std::endl;

    return 0;
}