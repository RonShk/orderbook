#pragma once
#include <iosfwd>
#include <string>
#include <vector>
#include "order.hpp"

// Text form of a side: "Buy" or "Sell". Reading anything else sets failbit.
std::ostream& operator<<(std::ostream& os, Side side);
std::istream& operator>>(std::istream& is, Side& side);

std::ostream& operator<<(std::ostream& os, const Order& ord);
std::ostream& operator<<(std::ostream& os, const Trade& trade);

// Reads "<Buy|Sell> <quantity> <price in cents>" lines from path into orders,
// giving ids 1, 2, 3 in file order. Prints the problem to stderr and returns
// false if the file can't be opened or a line can't be parsed.
bool read_orders(const std::string& path, std::vector<Order>& orders);
