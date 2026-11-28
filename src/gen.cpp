#include <algorithm>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>
#include "order.hpp"
#include "order_io.hpp"

// usage: gen <count> <output-file> [--cancel <pct>] [--modify <pct>] [--seed <n>]
//
// Writes count command lines. With the default 0% cancels and 0% modifies every
// line is an add, and the file is byte-identical to the v1 generator's output
// for the same count and seed (default 43), so data/orders_*.txt can be rebuilt.
//
// Cancels and modifies target one of the most recent adds, using the side that
// add had. Some targets will already have filled, which exercises NotFound.

constexpr int defaultSeed = 43;
constexpr int targetWindow = 1000;   // cancels and modifies pick one of the last this-many adds
constexpr int rejectPct = 2;         // share of modifies sent with quantity 0, which is rejected
constexpr int samePricePct = 50;     // share of modifies that keep the order's price

static void usage(const char* program) {
    std::cerr << "usage: " << program
              << " <count> <output-file> [--cancel <pct>] [--modify <pct>] [--seed <n>]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        usage(argv[0]);
        return 1;
    }

    int count = 0;
    int cancelPct = 0;
    int modifyPct = 0;
    int seed = defaultSeed;
    const std::string path = argv[2];

    try {
        count = std::stoi(argv[1]);
        for (int i = 3; i < argc; i += 2) {
            const std::string flag = argv[i];
            if (i + 1 >= argc) {
                std::cerr << "missing value for " << flag << '\n';
                return 1;
            }
            const int value = std::stoi(argv[i + 1]);
            if (flag == "--cancel") {
                cancelPct = value;
            } else if (flag == "--modify") {
                modifyPct = value;
            } else if (flag == "--seed") {
                seed = value;
            } else {
                std::cerr << "unknown flag: " << flag << '\n';
                return 1;
            }
        }
    } catch (const std::exception&) {
        usage(argv[0]);
        return 1;
    }

    if (count <= 0 || cancelPct < 0 || modifyPct < 0 || cancelPct + modifyPct > 100) {
        std::cerr << "count must be positive and the percentages must be 0-100 in total\n";
        return 1;
    }

    std::ofstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << path << '\n';
        return 1;
    }

    // Adds draw only from gen, in the same order as v1, so they don't depend on
    // the cancel/modify mix. Everything else draws from eventGen.
    std::mt19937 gen(seed);
    std::mt19937 eventGen(seed + 1);
    std::uniform_int_distribution<int> priceDist(9000, 11000);
    std::uniform_int_distribution<int> sideDist(0, 1);
    std::uniform_int_distribution<int> quantityDist(1, 100);
    std::uniform_int_distribution<int> percentDist(0, 99);
    std::uniform_int_distribution<int> priceShiftDist(-100, 100);

    // what the generator last sent for each add, indexed by id - 1
    std::vector<Side> sides;
    std::vector<int> prices;

    const auto pick_target = [&]() {
        const int issued = static_cast<int>(sides.size());
        const int window = std::min(targetWindow, issued);
        const int offset = std::uniform_int_distribution<int>(0, window - 1)(eventGen);
        return issued - offset;
    };

    for (int i = 0; i < count; i++) {
        const int roll = percentDist(eventGen);
        const bool haveTarget = !sides.empty();

        if (haveTarget && roll < cancelPct) {
            const int id = pick_target();
            file << "Cancel " << id << ' ' << sides[id - 1] << '\n';
        } else if (haveTarget && roll < cancelPct + modifyPct) {
            const int id = pick_target();
            const bool reject = percentDist(eventGen) < rejectPct;
            const int quantity = reject ? 0 : quantityDist(eventGen);
            int price = prices[id - 1];
            if (percentDist(eventGen) >= samePricePct) {
                price = std::max(1, price + priceShiftDist(eventGen));
            }
            if (!reject) {
                prices[id - 1] = price;
            }
            file << "Modify " << id << ' ' << sides[id - 1] << ' ' << quantity << ' ' << price << '\n';
        } else {
            const int price = priceDist(gen);
            const auto side = static_cast<Side>(sideDist(gen));
            const int quantity = quantityDist(gen);
            sides.push_back(side);
            prices.push_back(price);
            file << side << " " << quantity << " " << price << '\n';
        }
    }
}
