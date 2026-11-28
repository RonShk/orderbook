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
std::ostream& operator<<(std::ostream& os, CancelState state);
std::ostream& operator<<(std::ostream& os, ModifyState state);

// Reads one command per line from path into commands. Blank lines are skipped.
//
//   Buy <quantity> <price>                     add
//   Sell <quantity> <price>                    add
//   Cancel <id> <Buy|Sell>                     cancel a resting order
//   Modify <id> <Buy|Sell> <quantity> <price>  change a resting order's remaining
//                                              quantity and price
//
// Prices are in cents. Adds get ids 1, 2, 3 in file order; cancel and modify
// lines don't use up an id. Prints the problem to stderr and returns false if
// the file can't be opened or a line can't be parsed.
bool read_commands(const std::string& path, std::vector<Command>& commands);
