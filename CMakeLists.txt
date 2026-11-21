cmake_minimum_required(VERSION 4.4)
project(orderbook)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(orderbook
        src/main.cpp
        src/order_book.cpp)

add_executable(gen
        src/gen.cpp)