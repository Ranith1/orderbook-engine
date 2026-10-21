# Orderbook

This project is a central limit orderbook (CLOB) for one instrument, using price-time priority (best price first, then the earliest order at that price).


The orderbook is built using C++. It enables limit and market orders, along with cancel. 

# Running the Orderbook

```
g++ -std=c++20 -Wall -Wextra -I headers src/main.cpp src/orderbook.cpp -o orderbook
./orderbook sample/simulation.txt

g++ -std=c++20 -Wall -Wextra -I headers src/tests.cpp src/orderbook.cpp \
-lgtest_main -lgtest -pthread -o tests
./tests

```


