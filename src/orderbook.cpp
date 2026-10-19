#include "orderbook.h"
#include <algorithm>
#include <iterator>
#include <utility>

// addOrder

void Orderbook::addOrder(Order order) {

    // Cannot trade an order of 0
    if (order.quantity = 0) {
        return;
    }

    // An order with this ID already exists in the hash table
    if (orders_.contains(order.id)) {
        return;
    }

    // Match a trade with the other side as much as possible
    match(order);

    // Place leftover limit trades in the resting queue. 
    if (order.quantity > 0 && order.orderType == OrderType::Limit)
    {
        addResting(order);
    }
    

}

// cancelOrder
bool Orderbook::cancelOrder(OrderID id){

    // Locating the order
    auto orderEntry = orders_.find(id);
    if (orderEntry == orders_.end()) {
        return false;
    }

    const OrderLocation& location = orderEntry->second;

    // find the correct map, dependent on side order belonds to
    // use price to find the queue, then remove the order using the saved iterator

}
// Matching 
void Orderbook::match(Order& incoming) {
    return;
}

// addResting
void Orderbook::addResting(Order order){
    return;
}
