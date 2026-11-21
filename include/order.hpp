#pragma once 



enum class Side {
    Buy, Sell
};


struct Order {
    Side side;
    int quantity;
    int price;
    int id;
};

struct Trade {
    int buyID;
    int sellID;
    int priceTradedAt;
    int quantityTradedAt;
};
