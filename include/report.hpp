#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include "order.hpp"

// Running totals for one run, used to check a new version makes the same trades
// and gets the same cancel and modify results.
struct RunStats {
    std::size_t orders = 0;   // add commands
    std::uint64_t tradeCount = 0;
    std::uint64_t checksum = 0;

    // counts indexed by the result enum's value
    std::array<std::uint64_t, 2> cancelResults{};   // Canceled, NotFound
    std::array<std::uint64_t, 3> modifyResults{};   // Modified, NotFound, Rejected
};

// Counts the trade and folds its fields into stats.checksum.
void record_trade(RunStats& stats, const Trade& trade);

// Count the result and fold it into stats.checksum. A modify's trades are
// recorded separately with record_trade, after its result.
void record_cancel(RunStats& stats, int orderId, CancelState state);
void record_modify(RunStats& stats, int orderId, ModifyState state);

// Prints the "orders", "trades" and "checksum" lines. The "cancels" and
// "modifies" lines are printed only when the run had any, so a file of only
// adds gives the same output as v1.
void print_summary(const RunStats& stats);
