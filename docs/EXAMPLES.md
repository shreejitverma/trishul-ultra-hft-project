# Usage Examples

## 1. Using the Asynchronous Logger

The `AsyncLogger` allows you to log diagnostic information from critical trading threads without incurring the cost of disk I/O.

```cpp
#include "ultra/core/async_logger.hpp"

int main() {
    // 1. Start the logger background thread
    ultra::AsyncLogger::instance().start("trading_engine.log");

    // 2. Log from hot path (Non-blocking)
    // Uses C++11 Memory Barriers for synchronization
    ULTRA_LOG("Engine Initialized. Core Isolation: %s", "ACTIVE");

    for (int i = 0; i < 100; ++i) {
        // This pushes to a lock-free SPSC queue
        ULTRA_LOG("Processing Order ID: %d", i);
    }

    // 3. Cleanup on shutdown
    ultra::AsyncLogger::instance().stop();
    return 0;
}
```

## 2. Microstructure Simulation & Benchmarking

Trishul allows you to generate millions of trades using self-exciting point processes and stress-test your hardware limits.

### Generate 100M Hawkes Trades
```bash
# Command: <binary> <output_file> <num_events>
./build/data_generator market_data_100m.bin 100000000
```

### Empirical Throughput Stress Test
Run the tool to identify when your PCIe/DMA bus saturates.
```bash
./build/throughput_stress_test
```

## 3. Vectorized Signal Generation

Use the SIMD engine to process price vectors in batches.

```cpp
#include "ultra/strategy/signal_engine.hpp"
#include <vector>

void calculate_features() {
    std::vector<double> prices = {150.1, 150.2, 150.3, ...};
    
    // Computes SMA over the vector using AVX2 instructions
    auto sma = ultra::strategy::SignalEngine::sma(prices, 50);
}
```

## 4. Hardware Compliance Check

The `risk_gate.v` module ensures that every order sent by your AI policy is legally compliant.

```verilog
// Instantiate in your top-level RTL
risk_gate compliance_engine (
    .clk(sys_clk),
    .rst(sys_rst),
    .enable(1'b1),
    .notional_limit(64'd1000000), // $1M USD Limit
    .in_valid(order_valid),
    .in_px(order_price),
    .in_qty(order_qty),
    .pass(final_order_valid)
);
```
