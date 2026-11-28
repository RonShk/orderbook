#include "report.hpp"
#include <iostream>
#include <numeric>
#include "order_io.hpp"

// Folds one value into the running checksum. Order matters, so the same trades
// in a different sequence give a different checksum. uint64_t wraps on overflow,
// which is well defined for unsigned types.
static void mix(std::uint64_t& checksum, const int value) {
    checksum = checksum * 1000003 + static_cast<std::uint64_t>(value);
}

// Tags put in front of a cancel or modify result in the checksum, so the two
// kinds can't be mistaken for each other or for a trade's fields.
constexpr int cancelTag = 1;
constexpr int modifyTag = 2;

void record_trade(RunStats& stats, const Trade& trade) {
    ++stats.tradeCount;
    mix(stats.checksum, trade.buyID);
    mix(stats.checksum, trade.sellID);
    mix(stats.checksum, trade.priceTradedAt);
    mix(stats.checksum, trade.quantityTradedAt);
}

void record_cancel(RunStats& stats, const int orderId, const CancelState state) {
    const int value = static_cast<int>(state);
    ++stats.cancelResults[value];
    mix(stats.checksum, cancelTag);
    mix(stats.checksum, orderId);
    mix(stats.checksum, value);
}

void record_modify(RunStats& stats, const int orderId, const ModifyState state) {
    const int value = static_cast<int>(state);
    ++stats.modifyResults[value];
    mix(stats.checksum, modifyTag);
    mix(stats.checksum, orderId);
    mix(stats.checksum, value);
}

template <std::size_t N>
static std::uint64_t total(const std::array<std::uint64_t, N>& counts) {
    return std::accumulate(counts.begin(), counts.end(), std::uint64_t{0});
}

void print_summary(const RunStats& stats) {
    std::cout << "orders " << stats.orders << '\n';

    if (const std::uint64_t cancels = total(stats.cancelResults); cancels != 0) {
        std::cout << "cancels " << cancels
                  << " canceled=" << stats.cancelResults[static_cast<int>(CancelState::Canceled)]
                  << " not_found=" << stats.cancelResults[static_cast<int>(CancelState::NotFound)] << '\n';
    }

    if (const std::uint64_t modifies = total(stats.modifyResults); modifies != 0) {
        std::cout << "modifies " << modifies
                  << " modified=" << stats.modifyResults[static_cast<int>(ModifyState::Modified)]
                  << " not_found=" << stats.modifyResults[static_cast<int>(ModifyState::NotFound)]
                  << " rejected=" << stats.modifyResults[static_cast<int>(ModifyState::Rejected)] << '\n';
    }

    std::cout << "trades " << stats.tradeCount << '\n';
    std::cout << "checksum " << stats.checksum << '\n';
}
