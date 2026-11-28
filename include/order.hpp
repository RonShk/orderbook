#pragma once
#include <vector>


struct Trade {
    int buyID;
    int sellID;
    int priceTradedAt;
    int quantityTradedAt;
};

enum class Side {
    Buy, Sell
};

struct Order {
    Side side;
    int quantity;
    int price;
    int id;
};

// The checksum folds in the numeric values of these two enums, so don't reorder them.
enum class CancelState {
    Canceled, NotFound
};

enum class ModifyState {
    Modified, NotFound, Rejected
};

struct ModifyResult {
    ModifyState status{};
    std::vector<Trade> trades{};
};

enum class CommandType {
    Add, Cancel, Modify
};

// One line of an input file.
// Add:    order is the new order.
// Cancel: order.id and order.side name the order; price and quantity are unused.
// Modify: order.id and order.side name the order; price and quantity are the new values.
struct Command {
    CommandType type{};
    Order order{};
};
