#include "report.hpp"
#include <iostream>
#include "order_io.hpp"

// Folds one value into the running checksum. Order matters, so the same trades
// in a different sequence give a different checksum. uint64_t wraps on overflow,
// which is well defined for unsigned types.
static void mix(std::uint64_t& checksum, const int value) {
    checksum = checksum * 1000003 + static_cast<std::uint64_t>(value);
}

void record_trade(RunStats& stats, const Trade& trade) {
    ++stats.tradeCount;
    mix(stats.checksum, trade.buyID);
    mix(stats.checksum, trade.sellID);
    mix(stats.checksum, trade.priceTradedAt);
    mix(stats.checksum, trade.quantityTradedAt);
}

void print_trades(const std::vector<Trade>& trades) {
    for (const Trade& trade : trades) {
        std::cout << trade << '\n';
    }
}

void print_summary(const RunStats& stats) {
    std::cout << "orders " << stats.orders << '\n';
    std::cout << "trades " << stats.tradeCount << '\n';
    std::cout << "checksum " << stats.checksum << '\n';
}
