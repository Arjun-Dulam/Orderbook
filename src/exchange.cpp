#include "../include/exchange.hpp"
#include "../include/orderbook.hpp"

Exchange::Exchange() {

}

void Exchange::add_symbol(const std::string& symbol) {
    std::unique_lock<std::shared_mutex> lock(books_mutex_);
    books[symbol] = std::make_unique<OrderBook>();
}

std::vector<Trade> Exchange::add_order(std::string &symbol, Order &order) {
    OrderBook *book = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(books_mutex_);
        book = books[symbol].get();
    }
    return book->add_order(order);
}

bool Exchange::remove_order(std::string &symbol, uint32_t order_id) {
    OrderBook *book = nullptr;
    {
        std::shared_lock<std::shared_mutex> lock(books_mutex_);
        book = books[symbol].get();
    }

    return book->remove_order(order_id);
}