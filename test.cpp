#include <iostream>
#include "order.hpp"
#include "orderbook.hpp"

int main() {
    Order new_order = Order{1, Buy, 10, 10, 1};
    Order other_order = Order{2, Sell, 100, 100, 20};

    std::cout << new_order.order_id << side_to_string(new_order.side) << new_order.price << new_order.quantity << new_order.timestamp << '\n';
    std::cout << other_order.order_id << side_to_string(other_order.side) << other_order.price << other_order.quantity << other_order.timestamp << '\n';

    Trade new_trade = Trade{1, 100, 100, new_order.order_id, other_order.order_id};

    std::cout << new_trade.trade_id << new_trade.price << new_trade.quantity << new_trade.buy_order_id << new_trade.sell_order_id << '\n';
    return 0;
};

