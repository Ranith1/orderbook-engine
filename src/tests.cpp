
/*

Unit tests.

*/

#include "orderbook.h"
#include <gtest/gtest.h>
#include <cstdlib>
#include <optional>
#include <vector>

// Building orders shortcut
Order limit(Side side, Price price, uint64_t quantity) {
    return Order{0, price, OrderType::Limit, side, quantity};
}

Order market(Side side, uint64_t quantity) {
    return Order{0, 0, OrderType::Market, side, quantity};
}

// determiens whether there is an exact quantity of an instrument at the best price

bool bestLevelHasQuantity(Orderbook& book, Side side, uint64_t quantity) {

    std::optional<Price> best = (side == Side::Bid) ? book.bestBid() : book.bestAsk();
    if (!best) {
        return false;
    }

    Side opposite = (side == Side::Bid) ? Side::Ask : Side::Bid;
    book.addOrder(limit(opposite, *best, quantity));

    std::optional<Price> sameSide = (side == Side::Bid) ? book.bestBid() : book.bestAsk();
    std::optional<Price> otherSide = (side == Side::Bid) ? book.bestAsk() : book.bestBid();

    bool levelCleared = (sameSide != best);
    bool nothingLeftOver = (otherSide != best);
    return levelCleared && nothingLeftOver;

}

class OrderbookTest : public ::testing::Test {
protected:
    Orderbook book;
};

// Resting orders

TEST_F(OrderbookTest, EmptyBook) {
    EXPECT_EQ(book.bestBid(), std::nullopt);
    EXPECT_EQ(book.bestAsk(), std::nullopt);
}

TEST_F(OrderbookTest, LimitOrdersRestOnTheirSide) {
    book.addOrder(limit(Side::Bid, 99, 5));
    book.addOrder(limit(Side::Ask, 101, 3));

    EXPECT_EQ(book.bestBid(), 99);
    EXPECT_EQ(book.bestAsk(), 101);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Bid, 5));
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Ask, 3));
}

TEST_F(OrderbookTest, BestPricesAreHighestBidAndLowestAsk) {
    book.addOrder(limit(Side::Bid, 97, 1));
    book.addOrder(limit(Side::Bid, 99, 1));
    book.addOrder(limit(Side::Bid, 98, 1));
    book.addOrder(limit(Side::Ask, 103, 1));
    book.addOrder(limit(Side::Ask, 101, 1));
    book.addOrder(limit(Side::Ask, 102, 1));

    EXPECT_EQ(book.bestBid(), 99);
    EXPECT_EQ(book.bestAsk(), 101);
}

TEST_F(OrderbookTest, SamePriceOrdersAddUp) {
    book.addOrder(limit(Side::Bid, 100, 4));
    book.addOrder(limit(Side::Bid, 100, 6));

    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Bid, 10));
}

TEST_F(OrderbookTest, NegativePricesWork) {
    // negative pricing
    book.addOrder(limit(Side::Bid, -5, 2));
    book.addOrder(limit(Side::Ask, -3, 2));

    EXPECT_EQ(book.bestBid(), -5);
    EXPECT_EQ(book.bestAsk(), -3);
}

// IDs and rejects

TEST_F(OrderbookTest, IdsCountUpFromOne) {
    EXPECT_EQ(book.addOrder(limit(Side::Bid, 99, 1)), 1u);
    EXPECT_EQ(book.addOrder(limit(Side::Ask, 101, 1)), 2u);
    EXPECT_EQ(book.addOrder(market(Side::Bid, 1)), 3u);    
}

TEST_F(OrderbookTest, ZeroQuantityIsRejected) {
    EXPECT_EQ(book.addOrder(limit(Side::Bid, 99, 0)), std::nullopt);
    EXPECT_EQ(book.bestBid(), std::nullopt);

    EXPECT_EQ(book.addOrder(market(Side::Bid, 0)), std::nullopt);   

    // A rejected order doesn't use up an ID
    EXPECT_EQ(book.addOrder(limit(Side::Bid, 99, 1)), 1u);
}

// matching

TEST_F(OrderbookTest, CrossingLimitFullyFillsBoth) {
    book.addOrder(limit(Side::Ask, 100, 5));
    book.addOrder(limit(Side::Bid, 100, 5));

    EXPECT_EQ(book.bestBid(), std::nullopt);
    EXPECT_EQ(book.bestAsk(), std::nullopt);
}

TEST_F(OrderbookTest, IncomingBiggerThanRestingLeftoverRests) {
    book.addOrder(limit(Side::Ask, 100, 3));
    book.addOrder(limit(Side::Bid, 101, 10));   

    EXPECT_EQ(book.bestAsk(), std::nullopt);
    EXPECT_EQ(book.bestBid(), 101);             
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Bid, 7));
}

TEST_F(OrderbookTest, IncomingSmallerThanRestingRestingIsReduced) {
    book.addOrder(limit(Side::Ask, 100, 10));
    book.addOrder(limit(Side::Bid, 100, 4));

    EXPECT_EQ(book.bestBid(), std::nullopt);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Ask, 6));
}

TEST_F(OrderbookTest, LimitThatDoesNotCrossJustRests) {
    book.addOrder(limit(Side::Ask, 102, 5));
    book.addOrder(limit(Side::Bid, 101, 5));    

    EXPECT_EQ(book.bestBid(), 101);
    EXPECT_EQ(book.bestAsk(), 102);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Ask, 5));
}

TEST_F(OrderbookTest, LimitWalksLevelsButStopsAtItsPrice) {
    book.addOrder(limit(Side::Ask, 101, 2));
    book.addOrder(limit(Side::Ask, 102, 2));
    book.addOrder(limit(Side::Ask, 104, 2));
    book.addOrder(limit(Side::Bid, 102, 10));   

    EXPECT_EQ(book.bestAsk(), 104);
    EXPECT_EQ(book.bestBid(), 102);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Bid, 6));
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Ask, 2));
}

TEST_F(OrderbookTest, SellSideMatchesToo) {
    book.addOrder(limit(Side::Bid, 100, 2));
    book.addOrder(limit(Side::Bid, 99, 2));
    book.addOrder(limit(Side::Ask, 99, 3));     

    EXPECT_EQ(book.bestBid(), 99);
    EXPECT_EQ(book.bestAsk(), std::nullopt);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Bid, 1));
}

TEST_F(OrderbookTest, OldestOrderAtAPriceFillsFirst) {
    OrderID first = *book.addOrder(limit(Side::Ask, 100, 5));
    OrderID second = *book.addOrder(limit(Side::Ask, 100, 5));
    book.addOrder(limit(Side::Bid, 100, 5));   

    EXPECT_FALSE(book.cancelOrder(first));      
    EXPECT_TRUE(book.cancelOrder(second));      
}

TEST_F(OrderbookTest, BetterPriceBeatsEarlierTime) {
    OrderID earlierWorse = *book.addOrder(limit(Side::Ask, 101, 1));
    OrderID laterBetter = *book.addOrder(limit(Side::Ask, 100, 1));
    book.addOrder(limit(Side::Bid, 101, 1));    

    EXPECT_FALSE(book.cancelOrder(laterBetter));    
    EXPECT_TRUE(book.cancelOrder(earlierWorse));    
}

// market orders

TEST_F(OrderbookTest, MarketOrderWalksLevels) {
    book.addOrder(limit(Side::Ask, 101, 4));
    book.addOrder(limit(Side::Ask, 102, 3));
    book.addOrder(limit(Side::Ask, 105, 5));
    book.addOrder(market(Side::Bid, 10));       

    EXPECT_EQ(book.bestAsk(), 105);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Ask, 2));
}

TEST_F(OrderbookTest, MarketLeftoverIsCancelledNotRested) {
    book.addOrder(limit(Side::Ask, 101, 3));
    book.addOrder(market(Side::Bid, 10));       

    EXPECT_EQ(book.bestAsk(), std::nullopt);
    EXPECT_EQ(book.bestBid(), std::nullopt);   
}

TEST_F(OrderbookTest, MarketSellWalksBids) {
    book.addOrder(limit(Side::Bid, 100, 2));
    book.addOrder(limit(Side::Bid, 98, 2));
    book.addOrder(market(Side::Ask, 3));        

    EXPECT_EQ(book.bestBid(), 98);
    EXPECT_EQ(book.bestAsk(), std::nullopt);
    EXPECT_TRUE(bestLevelHasQuantity(book, Side::Bid, 1));
}

TEST_F(OrderbookTest, MarketOrderOnEmptyBook) {
    EXPECT_EQ(book.addOrder(market(Side::Ask, 5)), 1u);   
    EXPECT_EQ(book.bestBid(), std::nullopt);
    EXPECT_EQ(book.bestAsk(), std::nullopt);
}

// cancels

TEST_F(OrderbookTest, CancelRemovesOrder) {
    OrderID id = *book.addOrder(limit(Side::Bid, 99, 5));

    EXPECT_TRUE(book.cancelOrder(id));
    EXPECT_EQ(book.bestBid(), std::nullopt);
}

TEST_F(OrderbookTest, CancelPartiallyFilledOrder) {
    OrderID id = *book.addOrder(limit(Side::Ask, 100, 10));
    book.addOrder(limit(Side::Bid, 100, 4));    

    EXPECT_TRUE(book.cancelOrder(id));          
    EXPECT_EQ(book.bestAsk(), std::nullopt);
}

TEST_F(OrderbookTest, CancelLastOrderAtBestPriceMovesBestPrice) {
    book.addOrder(limit(Side::Ask, 102, 1));
    OrderID best = *book.addOrder(limit(Side::Ask, 101, 1));

    book.cancelOrder(best);
    EXPECT_EQ(book.bestAsk(), 102);
}

TEST_F(OrderbookTest, CancelUnknownOrTwiceFails) {
    EXPECT_FALSE(book.cancelOrder(42));

    OrderID id = *book.addOrder(limit(Side::Bid, 99, 5));
    EXPECT_TRUE(book.cancelOrder(id));
    EXPECT_FALSE(book.cancelOrder(id));
}

TEST_F(OrderbookTest, CancelFromMiddleKeepsQueueOrder) {
    OrderID a = *book.addOrder(limit(Side::Ask, 100, 1));
    OrderID b = *book.addOrder(limit(Side::Ask, 100, 1));
    OrderID c = *book.addOrder(limit(Side::Ask, 100, 1));

    EXPECT_TRUE(book.cancelOrder(b));
    book.addOrder(limit(Side::Bid, 100, 1));    

    EXPECT_FALSE(book.cancelOrder(a));          
    EXPECT_TRUE(book.cancelOrder(c));           
    EXPECT_EQ(book.bestAsk(), std::nullopt);
}
