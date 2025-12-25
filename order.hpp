#pragma once
#include <cstdint>

enum Side {
    Buy,
    Sell
};

struct Order {
    uint32_t order_id;
    Side side;
    int32_t price; // Price is stored as cents, choosing to have two decimal places of precision.
    uint32_t quantity;
    uint64_t timestamp;
};

struct Trade {
    uint32_t trade_id;
    int32_t price;
    uint32_t quantity;
    uint32_t buy_order_id;
    uint32_t sell_order_id;
};

std::string side_to_string(Side side) {
    if (side == Side::Buy) {
        return "Buy";
    } else {
        return "Sell";
    }
};