#include <fstream>
#include <random>
#include "../include/order.hpp"


static std::ostream& operator<<(std::ostream& os, Side side)
{
    // write obj to stream
    switch (side) {
        case Side::Buy: return os << "Buy";
        case Side::Sell: return os << "Sell";
    }
    return os << "Unknown";
}

int main() {
    std::ofstream file("orders_10k.txt");
    constexpr int orderNum = 10000;
    std::mt19937 gen(43);
    std::uniform_int_distribution<int> priceDist(9000, 11000);
    std::uniform_int_distribution<int> sideDist(0,1);
    std::uniform_int_distribution<int> quantityDist(1,100);


    for (int i = 0; i < orderNum; i++) {
        const int price = priceDist(gen);
        const auto side = static_cast<Side>(sideDist(gen));
        const int quantity = quantityDist(gen);
        
        file << side << " " << quantity << " " << price << '\n';
    }    

}
