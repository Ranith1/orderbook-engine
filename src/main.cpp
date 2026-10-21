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
        std::cout << "ask: none\n";
    }

}
// Turns sample data into an order
// False if the line isn't structured properly
bool processOrder(std::istringstream& in, Order& order) {
    
    std::string side, type;
    in >> side >> type >> order.price >> order.quantity;

    if (side == "BUY") {
        order.side = Side::Bid;

    } else if (side == "SELL") {
        order.side = Side::Ask;

    } else {
        return false;
    }

    if (type == "LIMIT") {
        order.orderType = OrderType::Limit;
    } else if (type == "MARKET") {
        order.orderType = OrderType::Market;
    } else {
        return false;
    }

    return true;
}


// mainn

int main(int argc, char* argv[]) {

    if (argc < 2) {
        return 1;
    }

    
    Orderbook book;
    std::string line;

    std::ifstream file(argv[1]);

    while (std::getline(file,line)) {

        std::istringstream in(line);
        std::string command;
        in >> command;

        if (command == "ADD") {

            Order order{};
            processOrder(in, order);
            book.addOrder(order);

        } else if(command == "CANCEL") {

            OrderID id;
            in >> id;
            book.cancelOrder(id); }


    }
    std::cout << "The best bid and ask: \n";
    printBestBidandBestAsk(book);


}