cmake_minimum_required(VERSION 3.21)

project(StockExchange VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCE test.cpp OrderBook.hpp)

add_executable(StockExchange ${SOURCE})
