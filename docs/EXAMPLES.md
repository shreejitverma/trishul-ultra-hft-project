# Usage Examples

## 1. Using the Asynchronous Logger

The `AsyncLogger` allows you to log diagnostic information from critical trading threads without incurring the cost of disk I/O.

```cpp
#include "ultra/core/async_logger.hpp"

int main() {
    // 1. Start the logger background thread
    ultra::AsyncLogger::instance().start("trading_engine.log");

    // 2. Log from hot path (Non-blocking)
    // Uses printf-style formatting
    ULTRA_LOG("Engine Initialized. Tick Size: %d", 1);

    for (int i = 0; i < 100; ++i) {
        // This pushes to a lock-free SPSC queue
        ULTRA_LOG("Processing Order ID: %d", i);
    }

    // 3. Cleanup on shutdown
    ultra::AsyncLogger::instance().stop();
    return 0;
}
```

## 2. Setting up a Market Making Strategy

This example demonstrates how to initialize the `MarketMaker` strategy, feed it market data, and retrieve generated orders.

```cpp
#include "ultra/strategy/market_maker.hpp"
#include "ultra/market-data/itch/decoder.hpp"

using namespace ultra;

int main() {
    // Initialize for Symbol ID 1 (e.g., AAPL)
    strategy::MarketMaker mm(1);

    // Simulate an ITCH Market Data Update (New Bid)
    md::itch::ITCHDecoder::DecodedMessage msg;
    msg.valid = true;
    msg.symbol_id = 1;
    msg.event_type = MDEventType::ADD_ORDER;
    msg.side = Side::BUY;
    msg.price = 1500000; // 150.0000
    msg.quantity = 100;

    // Feed strategy
    // This triggers the BBO update and potential quote generation
    mm.on_market_data(msg);

    // Check for generated orders
    strategy::StrategyOrder order;
    while (mm.get_order(order)) {
        std::cout << "Generated Order: " 
                  << (order.side == Side::BUY ? "BUY" : "SELL")
                  << " @ " << order.price << std::endl;
    }

    return 0;
}
```

## 3. High-Performance Packet Capture Replay

See `apps/md-replayer/` (Future Implementation) for how to use `libpcap` with `ultra::network::EthernetParser`.