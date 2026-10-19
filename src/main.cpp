/* Simulate orders through the orderbook */




#include "orderbook.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

void printBestBidandBestAsk(const Orderbook& book) {

    auto bid = book.bestBid();
    auto ask = book.bestAsk();

    if (bid) {
        std::cout << "bid: " << *bid << '\n';
    } else {
        std::cout << "bid: none\n";
    }

    if (ask) {
        std::cout << "ask: " << *ask << '\n';
    } else {
        std::cout << "bid: none\n";
    }

}


int main() {
    return 0;
}