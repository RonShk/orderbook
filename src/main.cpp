#include <chrono>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "order.hpp"
#include "order_book.hpp"
#include "order_io.hpp"
#include "report.hpp"

// usage: orderbook <commands-file> [--print-trades] [--print-book]
// stdout: (optional trades and cancel/modify results), summary, checksum, (optional book)
// stderr: time spent running the commands, so stdout stays identical between runs
int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <commands-file> [--print-trades] [--print-book]\n";
        return 1;
    }

    bool printTrades = false;
    bool printBook = false;
    for (int i = 2; i < argc; i++) {
        if (const std::string flag = argv[i]; flag == "--print-trades") {
            printTrades = true;
        } else if (flag == "--print-book") {
            printBook = true;
        } else {
            std::cerr << "unknown flag: " << flag << '\n';
            return 1;
        }
    }

    std::vector<Command> commands;
    if (!read_commands(argv[1], commands)) {
        return 1;
    }

    OrderBook book;
    RunStats stats;
    std::ostringstream log;   // only written with --print-trades, printed after timing stops

    const auto record_trades = [&](const std::vector<Trade>& trades) {
        for (const Trade& trade : trades) {
            record_trade(stats, trade);
            if (printTrades) {
                log << trade << '\n';
            }
        }
    };

    const auto start = std::chrono::steady_clock::now();
    for (const auto&[type, order] : commands) {
        const Order& ord = order;

        switch (type) {
            case CommandType::Add: {
                ++stats.orders;
                record_trades(book.add_order(ord));
                break;
            }
            case CommandType::Cancel: {
                const CancelState state = book.cancel_order(ord.id, ord.side);
                record_cancel(stats, ord.id, state);
                if (printTrades) {
                    log << "CANCEL id=" << ord.id << ' ' << state << '\n';
                }
                break;
            }
            case CommandType::Modify: {
                const ModifyResult result = book.modify_order(ord.id, ord.price, ord.quantity, ord.side);
                record_modify(stats, ord.id, result.status);
                if (printTrades) {
                    log << "MODIFY id=" << ord.id << ' ' << result.status << '\n';
                }
                record_trades(result.trades);
                break;
            }
        }
    }
    const auto end = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed_seconds = end - start;

    if (printTrades) {
        std::cout << log.str();
    }
    print_summary(stats);
    if (printBook) {
        book.print_order_book();
    }
    std::cerr << "seconds " << elapsed_seconds.count() << std::endl;

    return 0;
}
