#include "../include/exchange.hpp"

Exchange::Exchange() {

}

void Exchange::add_symbol(const std::string& symbol) {
    std::lock_guard<std::mutex> lock(books_mutex_);
    books[symbol] = std::make_unique<OrderBook>();
}

std::vector<Trade> Exchange::add_order(std::string &symbol, Order &order) {
    OrderBook *book = nullptr;
    {
        std::lock_guard<std::mutex> lock(books_mutex_);
        book = books[symbol].get();
    }
    return book->add_order(order);
}

bool Exchange::remove_order(std::string &symbol, uint32_t order_id) {
    Orderbook *book = nullptr;
    {
        std::lock_guard<std::mutex> lock(books_mutex_);
        book = books[symbol].get();
    }

    book.remove_order(order_id);
}