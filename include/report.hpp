#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include "order.hpp"

// Running totals for one run, used to check a new version makes the same trades.
struct RunStats {
    std::size_t orders = 0;
    std::uint64_t tradeCount = 0;
    std::uint64_t checksum = 0;
};

// Counts the trade and folds its fields into stats.checksum.
void record_trade(RunStats& stats, const Trade& trade);

void print_trades(const std::vector<Trade>& trades);

// Prints the "orders", "trades" and "checksum" lines.
void print_summary(const RunStats& stats);
