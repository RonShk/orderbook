#include "order_io.hpp"
#include <fstream>
#include <iostream>
#include <string>

std::ostream& operator<<(std::ostream& os, Side side) {
    switch (side) {
        case Side::Buy: return os << "Buy";
        case Side::Sell: return os << "Sell";
    }
    return os << "Unknown";
}

std::istream& operator>>(std::istream& is, Side& side) {
    std::string word;
    is >> word;

    if (word == "Buy") {
        side = Side::Buy;
    } else if (word == "Sell") {
        side = Side::Sell;
    } else {
        is.setstate(std::ios::failbit);
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const Order& ord) {
    return os << ord.side << " Price " << ord.price << " Quantity " << ord.quantity;
}

std::ostream& operator<<(std::ostream& os, const Trade& trade) {
    return os << "TRADE buy=" << trade.buyID << " sell=" << trade.sellID
              << " price=" << trade.priceTradedAt << " qty=" << trade.quantityTradedAt;
}

bool read_orders(const std::string& path, std::vector<Order>& orders) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << path << std::endl;
        return false;
    }

    Order order{};
    int price = 0;
    int quantity = 0;
    Side side = Side::Buy;
    int id = 0;

    while (file >> side >> quantity >> price) {
        order.side = side;
        order.price = price;
        order.quantity = quantity;
        order.id = ++id;
        orders.push_back(order);
    }

    // the loop stops at end of file or at a bad line; only end of file is ok
    if (!file.eof()) {
        std::cerr << "Bad input on line " << id + 1 << std::endl;
        return false;
    }

    return true;
}
