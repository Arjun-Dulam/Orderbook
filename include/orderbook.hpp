#pragma once
#include <map>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cstddef>

#include "order.hpp"

struct OrderLocation {
    Side side;
    int32_t price;
    size_t index;
};

class OrderBook {
private:
    std::map<int32_t, std::vector<Order>> asks; // Mapping price to array of corresponding asks
    std::map<int32_t, std::vector<Order>> bids; // Mapping price to array of corresponding bids
    std::unordered_map<uint32_t, OrderLocation> order_lookup; // Mapping order id to order location
    std::vector<Trade> trades;
    uint64_t next_timestamp;
    uint32_t next_trade_id;
    uint32_t next_order_id;

public:
    OrderBook();

    /**
     * @brief Adds order to order book, executes and returns possible trades
     * @param order Address of new order to be added to orderbook
     * @return Vector containing trades executed upon new order addition
     */
    std::vector<Trade> add_order(const Order &order);

    /**
     * @brief Executes possible trades given the addition of a new order
     * @param order Order that has recently been added
     * @param executed_trades Pointer to modifiable vector
     * @return Modified executed_trades vector containing executed trades
     */
    std::vector<Trade> init_trades_with_order(const Order *order, std::vector<Trade> *executed_trades);


    /**
     * @brief Scans and removes order if it exists
     * @param order_id order_id
     * @return True if order found and removed, false if order not found
     */
    bool remove_order(const uint32_t order_id);

    /**
     * @brief Shows executed trades
     * @return Vector containing all executed trades
     */
    std::vector<Trade> show_trades();

    /**
     * @brief Gets the optimal price for the corresponding side
     * @param map Map from which to obtain price
     * @param side Choosing which is the optimal value, greatest or smallest?
     * @return Lowest price if side is buy, highest price if side is sell
     */
    int32_t get_optimal_price(const std::map<int32_t, std::vector<Order>> &bids, const std::map<int32_t, std::vector<Order>> &asks, Side side);
};