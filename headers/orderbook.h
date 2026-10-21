/*

The Orderbook consists of three data structures:

- Bid: map of prices with a queue of orders, highest to lowest
- Ask: map of prices with a queue of orders, lowest to highest
- Orders: hash table of order IDs

*/



#pragma once
#include <map>
#include <list>
#include <functional>
#include <unordered_map>
#include "order.h"
#include <optional>

class Orderbook {

public:

/* 
Orderbook functionality - funtion declarations
*/

    // adding an order
    std::optional<OrderID> addOrder(Order order);
    // cancelling an order
    bool cancelOrder(OrderID id);

   

    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;

private: 

    // All of the Orders are a particular price
    using OrderQueue = std::list<Order>;

    /*
    
    A location is determined by which side its on (bid/ask),
    the price, and it's position in the order queue at that price.

    
    */
    struct OrderLocation {

        Side side;
        Price price;
        OrderQueue:: iterator it;

    };

    std::map<Price, OrderQueue, std::greater<Price>> bids_;
    std::map<Price, OrderQueue> asks_;

    // Cancelling orders
    std::unordered_map<OrderID, OrderLocation> orders_;

    // ID given to the next eligible order
    OrderID nextId_ = 1;

    void match(Order& incoming);
    void addResting(Order order);

};