#include "orderbook.hpp"

OrderBook::OrderBook() {
    next_timestamp = 0;
    next_trade_id = 0;
    next_order_id = 0;
}

std::vector<Trade> OrderBook::add_order(const Order &order) {
    Order new_order = order;
    new_order.timestamp = next_timestamp++;
    std::vector<Trade> executed_trades;

    init_trades_with_order(&new_order, &executed_trades);

    if (new_order.quantity == 0) {
        return executed_trades;
    }

    // Add order to orderbook if order not completely satisfied

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
}

void OrderBook::init_trades_with_order(const Order *order, std::vector<Trade> *executed_trades) {
    int32_t optimal_existing_price = get_optimal_price(bids, asks, order->side);
    bool trade_possible;

    Order oldest_bid = std::prev(bids.end())->second[0];
    Order oldest_ask = asks.begin()->second[0];
    Order existing_order = (order->side == Side::Buy) ? oldest_ask : oldest_bid;

    if (order->side == Side::Buy) {
        trade_possible = optimal_existing_price <= order->price;
    } else {
        trade_possible = optimal_existing_price >= order->price;
    }

    trade_possible = trade_possible && (order->quantity == existing_order.quantity);

    if (trade_possible) {
        Trade new_trade {
            next_trade_id++,
            optimal_existing_price,
            (order->side == Side::Buy) ? order->order_id : existing_order.order_id,
            (order->side == Side::Sell) ? order->order_id : existing_order.order_id,
        };

        trades.push_back(new_trade);
        executed_trades->push_back(new_trade);

        std::erase()
    }
}

bool OrderBook::remove_order(const uint32_t order_id) {
    // TODO
    return false;
}

std::vector<Trade> OrderBook::show_trades() {
    return trades;
}

int32_t OrderBook::get_optimal_price(const std::map<int32_t, std::vector<Order>> &bids, const std::map<int32_t, std::vector<Order>> &asks, Side side) {
    if (side == Side::Buy) {
        return asks.begin()->first;
    } else {
        return std::prev(bids.end())->first;
    }
}