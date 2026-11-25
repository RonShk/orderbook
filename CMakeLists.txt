cmake_minimum_required(VERSION 4.4)
project(orderbook)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(orderbook
        src/main.cpp
        src/order_book.cpp
        src/order_io.cpp
        src/report.cpp)

add_executable(gen
        src/gen.cpp
        src/order_io.cpp)