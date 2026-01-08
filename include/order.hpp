#pragma once

#include <string>
#include <cstdint>

enum class Side : uint8_t {
    Buy,
    Sell
};

struct Order {
    uint64_t timestamp;
    uint32_t order_id;
    int32_t price;
    // Price is stored as cents, choosing to have two decimal places of precision.
    // Using signed int to accommodate negative prices for commodities such as what happened to oil in 2020.
    uint32_t quantity;
    Side side;
    bool filled = false;
};

struct Trade {
    uint32_t trade_id;
    int32_t price;
    uint32_t quantity;
    uint32_t buy_order_id;
    uint32_t sell_order_id;
};

std::string side_to_string(const Side side);