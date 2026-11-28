#include "order_io.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

std::ostream& operator<<(std::ostream& os, Side side) {
    switch (side) {
        case Side::Buy: return os << "Buy";
        case Side::Sell: return os << "Sell";
    }
    return os << "Unknown";
}

std::istream& operator>>(std::istream& is, Side& side) {
    std::string word;
    is >> word;

    if (word == "Buy") {
        side = Side::Buy;
    } else if (word == "Sell") {
        side = Side::Sell;
    } else {
        is.setstate(std::ios::failbit);
    }

    return is;
}

std::ostream& operator<<(std::ostream& os, const Order& ord) {
    return os << ord.side << " Price " << ord.price << " Quantity " << ord.quantity;
}

std::ostream& operator<<(std::ostream& os, const Trade& trade) {
    return os << "TRADE buy=" << trade.buyID << " sell=" << trade.sellID
              << " price=" << trade.priceTradedAt << " qty=" << trade.quantityTradedAt;
}

std::ostream& operator<<(std::ostream& os, const CancelState state) {
    switch (state) {
        case CancelState::Canceled: return os << "Canceled";
        case CancelState::NotFound: return os << "NotFound";
    }
    return os << "Unknown";
}

std::ostream& operator<<(std::ostream& os, ModifyState state) {
    switch (state) {
        case ModifyState::Modified: return os << "Modified";
        case ModifyState::NotFound: return os << "NotFound";
        case ModifyState::Rejected: return os << "Rejected";
    }
    return os << "Unknown";
}

// Parses the fields after the first word of a line into command.
// Returns false if they are missing, malformed, or followed by anything else.
static bool parse_fields(const std::string& word, std::istream& fields, Command& command) {
    auto&[side, quantity, price, id] = command.order;

    if (word == "Buy" || word == "Sell") {
        command.type = CommandType::Add;
        side = (word == "Buy") ? Side::Buy : Side::Sell;
        fields >> quantity >> price;
    } else if (word == "Cancel") {
        command.type = CommandType::Cancel;
        fields >> id >> side;
    } else if (word == "Modify") {
        command.type = CommandType::Modify;
        fields >> id >> side >> quantity >> price;
    } else {
        return false;
    }

    if (fields.fail()) {
        return false;
    }

    std::string extra;
    return !(fields >> extra);
}

bool read_commands(const std::string& path, std::vector<Command>& commands) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Error opening file " << path << std::endl;
        return false;
    }

    std::string line;
    std::istringstream fields;   // reused for every line to avoid constructing a stream each time
    int lineNumber = 0;
    int nextId = 0;

    while (std::getline(file, line)) {
        ++lineNumber;
        fields.clear();
        fields.str(line);

        std::string word;
        if (!(fields >> word)) {
            continue;   // blank line
        }

        Command command{};
        if (!parse_fields(word, fields, command)) {
            std::cerr << "Bad input on line " << lineNumber << ": " << line << std::endl;
            return false;
        }

        if (command.type == CommandType::Add) {
            command.order.id = ++nextId;
        }
        commands.push_back(command);
    }

    // getline stops at end of file or on a read error; only end of file is ok
    if (file.bad()) {
        std::cerr << "Error reading file " << path << std::endl;
        return false;
    }

    return true;
}
