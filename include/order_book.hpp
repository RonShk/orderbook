#pragma once 
#include <vector>
#include "order.hpp"


class OrderBook {
private: 
    std::vector<Order> bids; //buys 
    std::vector<Order> asks; // sells
    

    void fill_buy_order(Order& orderToFill, std::vector<Trade>& trades);
    void fill_sell_order(Order& orderToFill, std::vector<Trade>& trades);
    void insert_resting_order(const Order& ord);

public:
    std::vector<Trade> add_order(const Order& ord);
    void print_order_book(); 

};
