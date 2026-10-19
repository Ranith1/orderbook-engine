#include "orderbook.h"
#include <algorithm>
#include <iterator>
#include <utility>

// addOrder

void Orderbook::addOrder(Order order) {

    // Cannot trade an order of 0
    if (order.quantity == 0) {
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
    
    if (location.side == Side::Bid) {

        auto levelIterator = bids_.find(location.price);
        levelIterator->second.erase(location.it);

        if (levelIterator->second.empty()){
            bids_.erase(levelIterator);
        }

    } else {
        auto levelIterator = asks_.find(location.price);
        levelIterator->second.erase(location.it);

        if(levelIterator->second.empty()) {
            asks_.erase(levelIterator);
        }
    }

    // Removing the cancelled order from the hash table
    orders_.erase(orderEntry);
    return true;

}
// Matching 
void Orderbook::match(Order& incoming) {
    
    auto matchAgainst = [&](auto& otherSide, auto canTrade){
    
        while (incoming.quantity > 0 && !otherSide.empty()) {

            // Best price on the other side
            auto levelIterator = otherSide.begin();
            Price levelPrice = levelIterator->first;

            // Limit orders stop filling at undesired prices
            if (incoming.orderType == OrderType::Limit && !canTrade(levelPrice)) {
                break;
            }

            OrderQueue& queue = levelIterator->second;

            // Filling the oldest orders
            while (incoming.quantity > 0 && !queue.empty()) {

                Order& resting = queue.front();
                uint64_t tradeQty = std::min(incoming.quantity, resting.quantity);

                incoming.quantity -= tradeQty;
                resting.quantity -= tradeQty;

                // Remove a fully filled order from the hash table.
                if (resting.quantity == 0) {
                    orders_.erase(resting.id);
                    queue.pop_front();
                }
            }

            // Remove the level once there are no ideas left at this price.
            if (queue.empty()) {
                otherSide.erase(levelIterator);
            }

            
        }


    };

    //  A buy trading against an ask is fine if ask <= buy limit
    // A sell trading against a bid is fine if bid >= sell limit
    if (incoming.side == Side::Bid) {
        matchAgainst(asks_, [&](Price askPrice) {
            return askPrice <= incoming.price;
        });
    } else {
        matchAgainst(bids_, [&](Price bidPrice) {
            return bidPrice >= incoming.price;
        });
    }


}

// addResting
// Placing leftover (partially filled) limit orders into the book, then recording it in the hash table
void Orderbook::addResting(Order order){
    
    // Saving information
    Side side = order.side;
    Price price = order.price;
    OrderID id = order.id;

    OrderQueue& queue = (side == Side::Bid) ? bids_[price] : asks_[price];

    // Newestr order goes to the back 
    queue.push_back(std::move(order));

    orders_.emplace(id, OrderLocation{side, price, std::prev(queue.end())});


}

std::optional<Price> Orderbook::bestBid() const {

    if (bids_.empty()) {
        return std::nullopt;
    }

    return bids_.begin() -> first;

}

std::optional<Price> Orderbook::bestAsk() const {

    if (asks_.empty()) {
        return std::nullopt;
    }

    return asks_.begin()->first;

}