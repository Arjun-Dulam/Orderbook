#include "orderbook.hpp"

#include <signal.h>

OrderBook::OrderBook() {
    next_timestamp = 0;
    next_trade_id = 0;
    next_order_id = 0;
}

std::vector<Trade> OrderBook::add_order(const Order &order) {
    Order new_order = order;
    new_order.timestamp = next_timestamp++;
    new_order.order_id = next_order_id++;
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

    return executed_trades;
}

void OrderBook::init_trades_with_order(Order *order, std::vector<Trade> *executed_trades) {
    while (order->quantity > 0) {
        if (order->side == Side::Buy && asks.empty()) {
            return;
        } else if (order->side == Side::Sell && bids.empty()) {
            return;
        }

        int32_t optimal_existing_price;
        Order *existing_order;

        if (order->side == Side::Buy) {
            optimal_existing_price = asks.begin()->first;
            existing_order = &asks.begin()->second[0];
        } else {
            optimal_existing_price = std::prev(bids.end())->first;
            existing_order = &std::prev(bids.end())->second[0];
        }

        bool trade_possible;

        if (order->side == Side::Buy) {
            trade_possible = optimal_existing_price <= order->price;
        } else {
            trade_possible = optimal_existing_price >= order->price;
        }

        if (!trade_possible) {
            break;
        }

        Trade new_trade {
            next_trade_id++,
            optimal_existing_price,
            std::min(order->quantity, existing_order->quantity),
            (order->side == Side::Buy) ? order->order_id : existing_order->order_id,
            (order->side == Side::Sell) ? order->order_id : existing_order->order_id
        };

        order->quantity -= new_trade.quantity;
        existing_order->quantity -= new_trade.quantity;
        if (existing_order->quantity == 0) {
            remove_order(existing_order->order_id);
        }

        trades.push_back(new_trade);
        executed_trades->push_back(new_trade);
    }
}

bool OrderBook::remove_order(uint32_t order_id) {
    if (!order_lookup.contains(order_id)) {
        return false;
    }

    OrderLocation order_location = order_lookup[order_id];
    auto *target_map = (order_location.side == Side::Buy) ? &bids : &asks;
    auto *orders_at_price = & (*target_map)[order_location.price];

    orders_at_price->erase(orders_at_price->begin() + order_location.index);

    for (size_t i = order_location.index; i < orders_at_price->size(); i++) {
        order_lookup[(*orders_at_price)[i].order_id].index = i;
    }

    order_lookup.erase(order_id);

    if (orders_at_price->empty()) {
        target_map->erase(order_location.price);
    }
    return true;
}

std::vector<Trade> OrderBook::show_trades() {
    return trades;
}