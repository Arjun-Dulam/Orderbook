#pragma once

#include "../include/order.hpp"
#include <random>
#include <vector>
#include <cstdint>

struct MarketConfig {
    int32_t base_price = 10000;
    double price_std_dev = 100;
    double arrival_rate = 1000;
    double cancel_rate = .30;
    uint32_t min_quantity = 1;
    uint32_t max_quantity = 10000;
    double power_law_alpha = 2.5;
    double buy_sell_ratio = 0.5;
};