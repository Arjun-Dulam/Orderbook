#pragma once
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>
#include <memory>

#include "orderbook.hpp"

class Exchange {
private:
    std::unordered_map<std::string, std::unique_ptr<OrderBook>> books;
    std::mutex books_mutex_;

public:
    Exchange();

    /**
     *
     * @param symbol the symbol to be added to orderbook
     * @brief allocates new orderbook on heap, wraps up in smart pointer, stores unique_ptr in u_map with symbol as key
     *
     */
    void add_symbol(const std::string& symbol);

    std::vector<Trade> add_order(std::string &symbol, Order &order);

    bool remove_order(std::string &symbol, uint32_t order_id);
};
