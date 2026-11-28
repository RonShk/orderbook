#include "../include/order_book.hpp"
#include <vector> 
#include <algorithm>
#include <iostream>
#include <iomanip>


void OrderBook::fill_sell_order(Order& orderToFill, std::vector<Trade>& trades) {
    Order& restingOrder = bids.back();
    const int quantityToFill = std::min(orderToFill.quantity, restingOrder.quantity);
    const Trade trade = {restingOrder.id, orderToFill.id, restingOrder.price, quantityToFill};
    trades.push_back(trade); 

    orderToFill.quantity -= quantityToFill;
    restingOrder.quantity -= quantityToFill; 

    if (restingOrder.quantity == 0) {
        bids.pop_back();
    }

}

void OrderBook::fill_buy_order(Order& orderToFill, std::vector<Trade>& trades) {
    Order& restingOrder = asks.back(); 
    const int quantityToFill = std::min(orderToFill.quantity, restingOrder.quantity);
    const Trade trade = {orderToFill.id, restingOrder.id, restingOrder.price, quantityToFill};
    trades.push_back(trade);

    orderToFill.quantity -= quantityToFill; 
    restingOrder.quantity -= quantityToFill; 

    if (restingOrder.quantity == 0) {
        asks.pop_back(); 
    }

}



//places a resting order in the book (was not able to get filled)
void OrderBook::insert_resting_order(const Order& ord) {
    if (ord.side == Side::Buy) {
        const auto pos = std::lower_bound(bids.begin(), bids.end(), ord.price,
            [](const Order& order, const int price) {
                return order.price < price;
        });

        bids.emplace(pos, ord);
    } else {
        const auto pos = std::lower_bound(asks.begin(), asks.end(), ord.price,
            [](const Order& order, const int price) {
                return order.price > price;
        });
        asks.emplace(pos, ord);
    }
}

std::vector<Trade> OrderBook::add_order(const Order& ord) {
    std::vector<Trade> trades;
    Order ordCopy = ord;  //copy of ord object to fill order if needed
    //                      dont use the original as to not mess up the order given by client
    

    if (ordCopy.side == Side::Sell) {
        while (!bids.empty() && bids.back().price >= ordCopy.price && ordCopy.quantity != 0) {
            fill_sell_order(ordCopy,trades); 
        }

        if (ordCopy.quantity != 0) {
            insert_resting_order(ordCopy);
        }
    } 

    if (ordCopy.side == Side::Buy){
        while (!asks.empty() && asks.back().price <= ordCopy.price && ordCopy.quantity != 0) {
            fill_buy_order(ordCopy, trades); 
        }

        if (ordCopy.quantity != 0) {

            insert_resting_order(ordCopy);
        }
    }

    return trades; 
};



void OrderBook::print_order_book() {
    std::cout << "ASKS\n";
    for (const auto& ask : asks) {   // descending, so the best ask ends up nearest the spread
        std::cout << "  " << ask.price / 100 << '.'
                  << std::setw(2) << std::setfill('0') << ask.price % 100
                  << std::setfill(' ') << "  x " << ask.quantity << '\n';
    }

    std::cout << "--- spread ---\n";

    std::cout << "BIDS\n";
    for (auto it = bids.rbegin(); it != bids.rend(); ++it) {   // best bid first
        std::cout << "  " << it->price / 100 << '.'
                  << std::setw(2) << std::setfill('0') << it->price % 100
                  << std::setfill(' ') << "  x " << it->quantity << '\n';
    }
}
//the best buyer is the one with the highest price so put that at the back
//the best seller is the one with the lowest price so put that at the back 


ModifyResult OrderBook::modify_order(const int orderId, const int newPrice, const int newQuantity, const Side side) {
    if (newPrice <= 0 || newQuantity <= 0) {
        return {ModifyState::Rejected, {}};
    }

    std::vector<Order>& book = (side == Side::Buy) ? bids : asks;
    const auto iter = std::find_if(book.begin(), book.end(), [orderId](const Order& order) {
        return order.id == orderId;
    });

    if (iter == book.end()) {
        return {ModifyState::NotFound, {}};
    }

    Order& orderFound = *iter;
    if (orderFound.quantity == newQuantity && orderFound.price == newPrice) {
        return {ModifyState::Modified, {}};
    }

    if (orderFound.price == newPrice && orderFound.quantity > newQuantity) {
        orderFound.quantity = newQuantity;
        return {ModifyState::Modified, {}};
    }

    auto orderToBeMatched = orderFound;
    orderToBeMatched.quantity = newQuantity;
    orderToBeMatched.price = newPrice;
    book.erase(iter);
    return {ModifyState::Modified, add_order(orderToBeMatched)};
}

CancelState OrderBook::cancel_order(const int orderId, const Side side) {
    std::vector<Order>& book = (side == Side::Buy) ? bids : asks;

    const auto it = std::find_if(book.begin(), book.end(),
                                 [orderId](const Order &order) {
                                     return order.id == orderId;
                                 });

    if (it == book.end()) {
        return CancelState::NotFound;
    }

    book.erase(it);
    return CancelState::Canceled;

}
