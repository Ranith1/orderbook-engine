#pragma once
#include <cstdint>

/* 

Properties of an order:

- The Side (Bid/Ask)
- Price
- Identifier (Order ID)
- Type (Limit/Market)

*/

using Price = int64_t;
using OrderID = uint64_t;

enum class OrderType {

    Limit,
    Market

};

enum class Side {

    Bid,
    Ask

};

struct Order {

    OrderID id;
    Price price;
    OrderType orderType;
    Side side;
    uint64_t quantity;

};