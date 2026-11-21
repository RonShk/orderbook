#include <iostream>
#include "../include/order.hpp"
#include "../include/order_book.hpp"
#include <istream>
#include <fstream>

static std::ostream& operator<<(std::ostream& os, Side sd) {
    if (const int val = static_cast<int>(sd); val == 0) {
        os << "Buy ";
    } else {
        os << "Sell";
    }

    return os;
}

static std::ostream& operator<<(std::ostream& os, const Order& ord) {
    os << ord.side << "Price " << ord.price << " Quantity " << ord.quantity << '\n';

    return os;
}


static  std::istream& operator>>(std::istream& is, Side& sd) {
    std::string word;
    is >> word; 

    if (word == "Buy") {
        sd = Side::Buy;
    } else if (word == "Sell") {
        sd = Side::Sell;
    } else {
        is.setstate(std::ios::failbit);
    }

    return is;
}

int main() {
    OrderBook book; 
    Order order{};
    int price; 
    int quantity; 
    Side side; 
    int id = 0;


    std::ifstream file("orders_1m.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }



    std::vector<Order> orders;
    while (file >> side >>quantity >> price ) {
        order.side = side;
        order.price = price;
        order.quantity = quantity;
        order.id = ++id;
        orders.push_back(order);
    }

    const auto start = std::chrono::steady_clock::now();
    for (const auto& ord : orders) {
        book.add_order(ord);
    }
    const auto end = std::chrono::steady_clock::now();

    const std::chrono::duration<double> elapsed_seconds = end - start;
    double seconds = elapsed_seconds.count();

    book.print_order_book();
    std::cout << "Took " << seconds << " seconds" << std::endl;

    return 0;
}
