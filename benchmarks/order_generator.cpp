#include "order_generator.hpp"

#include <algorithm>
#include <cmath>
#include <complex>

OrderGenerator::OrderGenerator(MarketConfig cfg, uint32_t seed) :
    config(cfg),
    rng(seed),
    price_dist(0.0, cfg.price_std_dev),
    uniform_dist(0.0, 1.0),
    arrival_dist(1.0 / cfg.arrival_rate),
    side_dist(cfg.buy_sell_ratio),
    cancel_dist(cfg.cancel_rate)
{
}

int32_t OrderGenerator::generate_price() {
    return price_dist(rng) + config.base_price;
}

uint32_t OrderGenerator::generate_quantity() {
    // Formula -> x = [(x_max^(pow) - x_min^(pow)) * y_unif + x_min^(pow)] ^ (1 / pow)
    double max_pow = std::pow(config.max_quantity, config.power_law_alpha);
    double min_pow = std::pow(config.min_quantity, config.power_law_alpha);

    double inner = ((max_pow - min_pow) * uniform_dist(rng) + min_pow);

    return uint32_t(pow(inner, 1 / config.power_law_alpha));
    
    /**
     * TODO: This sucks ass. Calling pow() too much. Optimize by precomputing, then computing again
     * only when config changes. Good enough for now.
    */
}

Side OrderGenerator::generate_side() {
    return (side_dist(rng)) ? Side::Buy : Side::Sell;
}

bool OrderGenerator::should_cancel() {
    return (cancel_dist(rng));
}

Order OrderGenerator::generate_order() {
    Order new_order = {
        .timestamp = 0,
        .order_id = 0,
        .price = generate_price(),
        .quantity = generate_quantity(),
        .side = generate_side(),
        .deleted_or_filled = false
    };

    return new_order;
}