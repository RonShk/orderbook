#include <fstream>
#include <random>
#include "order.hpp"
#include "order_io.hpp"


int main() {
    std::ofstream file("orders_1k.txt");
    constexpr int orderNum = 1000;
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
