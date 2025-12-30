#pragma once
#include <map>
#include <vector>
#include <unordered_map>
#include <cstdint>

#include "order.hpp"

struct OrderLocation {
    Side side;
    int32_t price;
    size_t index;
};

class OrderBook {
private:
    std::map<int32_t, std::vector<Order>> bids; // Mapping price to array of corresponding bids
    std::map<int32_t, std::vector<Order>> asks; // Mapping price to array of corresponding asks
    std::unordered_map<uint32_t, OrderLocation> order_lookup; // Mapping order id to order location
    std::vector<Trade> trades;
    uint64_t next_timestamp;

public:
    OrderBook();

    /**
     * @brief Adds order to order book, executes and returns possible trades
     * @param order Address of new order to be added to oderbook
     * @return Vector containing trades executed upon new order addition
     */

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
};