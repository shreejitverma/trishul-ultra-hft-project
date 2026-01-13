# Examples

## 1. Adding a Custom Indicator

To add a new indicator (e.g., Bollinger Bands) to the `SignalEngine`:

1.  Open `include/ultra/strategy/signal_engine.hpp`.
2.  Add a static method utilizing `SIMDUtils`.

```cpp
// Inside SignalEngine class
static std::pair<std::vector<double>, std::vector<double>> bollinger_bands(
    const std::vector<double>& prices, int window, double num_std_dev) 
{
    size_t n = prices.size();
    std::vector<double> upper(n), lower(n);
    std::vector<double> sma_vec = sma(prices, window);
    
    // Naive implementation (Vectorize this for production!)
    for (size_t i = window; i < n; ++i) {
        // Extract window for std dev
        std::vector<double> window_slice(prices.begin() + i - window, prices.begin() + i);
        double std = SIMDUtils::calculate_std_dev(window_slice);
        
        upper[i] = sma_vec[i] + (std * num_std_dev);
        lower[i] = sma_vec[i] - (std * num_std_dev);
    }
    return {upper, lower};
}
```

## 2. Implementing a Custom Strategy

Create a new class inheriting from `IStrategy` (conceptually, or just modify the `RLPolicyStrategy` for now as `IStrategy` is a virtual interface in `strategy.hpp`).

```cpp
#include "ultra/strategy/strategy.hpp"

class MomentumStrategy {
public:
    void on_market_data(const md::itch::ITCHDecoder::DecodedMessage& msg) {
        // 1. Update internal book
        book_.update(msg);
        
        // 2. Check signal
        if (signal_engine_.should_buy(book_.best_bid().price)) {
            // 3. Place Order
            StrategyOrder order;
            order.side = Side::BUY;
            order.price = book_.best_ask().price; // Crossing spread
            order.quantity = 100;
            // push to queue...
        }
    }
private:
    md::OrderBookL2 book_{1};
    MySignalEngine signal_engine_;
};
```

## 3. Replaying a PCAP File

To feed real captured data into the system, you can modify `tools/data-generator` to read a PCAP instead of generating random numbers.

1.  Use `libpcap` to open the file.
2.  Extract the payload from the UDP packets.
3.  Write the raw ITCH messages to `market_data.bin` in the same format the `data_generator` currently does.
4.  Run `strategy_backtester` on this new binary file.
