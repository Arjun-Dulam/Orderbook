#include "orderbook.hpp"

OrderBook::OrderBook(): next_timestamp (0)  {}

std::vector<Trade> OrderBook::add_order(const Order &order) {
    Order new_order = order;
    new_order.timestamp = next_timestamp++;

    std::map<int32_t, std::vector<Order>> *target_map;
    if (new_order.side == Side::Buy) {
        target_map = &bids;
    } else {
        target_map = &asks;
    }

    (*target_map)[new_order.price].push_back(new_order);

    size_t index = size((*target_map)[new_order.price]) - 1;
    OrderLocation order_location{new_order.side, new_order.price, index};
    order_lookup[new_order.order_id] = order_location;

    return {};
}

bool OrderBook::remove_order(const uint32_t order_id) {
    // TODO
    return false;
}

std::vector<Trade> OrderBook::show_trades() {
    return trades;
}



