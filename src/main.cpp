#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include "order.hpp"
#include "order_book.hpp"
#include "order_io.hpp"
#include "report.hpp"

// usage: orderbook <orders-file> [--print-trades] [--print-book]
// stdout: (optional trades), order count, trade count, checksum, (optional book)
// stderr: time spent matching, so stdout stays identical between runs
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <orders-file> [--print-trades] [--print-book]\n";
        return 1;
    }

    bool printTrades = false;
    bool printBook = false;
    for (int i = 2; i < argc; i++) {
        const std::string flag = argv[i];
        if (flag == "--print-trades") {
            printTrades = true;
        } else if (flag == "--print-book") {
            printBook = true;
        } else {
            std::cerr << "unknown flag: " << flag << '\n';
            return 1;
        }
    }

    std::vector<Order> orders;
    if (!read_orders(argv[1], orders)) {
        return 1;
    }

    OrderBook book;
    RunStats stats;
    stats.orders = orders.size();
    std::vector<Trade> allTrades;   // only filled with --print-trades

    const auto start = std::chrono::steady_clock::now();
    for (const auto& ord : orders) {
        const std::vector<Trade> trades = book.add_order(ord);
        for (const Trade& trade : trades) {
            record_trade(stats, trade);
        }
        if (printTrades) {
            allTrades.insert(allTrades.end(), trades.begin(), trades.end());
        }
    }
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed_seconds = end - start;

    if (printTrades) {
        print_trades(allTrades);
    }
    print_summary(stats);
    if (printBook) {
        book.print_order_book();
    }
    std::cerr << "seconds " << elapsed_seconds.count() << std::endl;

    return 0;
}
