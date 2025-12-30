#include "orderbook.hpp"

OrderBook::OrderBook() {
    next_timestamp {0};
}

std::vector<Trade> OrderBook::add_order(const Order &order) {
    // TODO
    return {};
}

bool OrderBook::remove_order(const uint32_t order_id) {
    // TODO
    return false;
}

std::vector<Trade> OrderBook::show_trades() {
    return trades;
}



