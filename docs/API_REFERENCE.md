# API Reference

This document outlines the core C++ interfaces for developers extending the Trishul Ultra-HFT platform.

## 1. Strategy & Signals

### `ultra::strategy::RLPolicyStrategy`
Located in `include/ultra/strategy/rl-inference/rl_policy.hpp`.
Implements the trading logic using a hybrid AI/Heuristic approach.

*   `void on_market_data(const DecodedMessage& msg)`
    *   Callback for new ITCH messages. Updates internal book and triggers inference.
*   `void run_inference()`
    *   Executes the Avellaneda-Stoikov logic to determine quote skew based on inventory.
*   `void on_execution(const ExecutionReport& report)`
    *   Callback for fill reports to update inventory state.

### `ultra::strategy::SignalEngine`
Located in `include/ultra/strategy/signal_engine.hpp`.
Provides SIMD-accelerated technical indicators.

*   `static std::vector<double> sma(const std::vector<double>& prices, int window)`
    *   Computes Simple Moving Average.
*   `static std::vector<double> rsi(const std::vector<double>& prices, int period)`
    *   Computes Relative Strength Index using rolling averages.

---

## 2. Execution System

### `ultra::execution::OrderManagementSystem`
Located in `include/ultra/execution/oms/oms_core.hpp`.
Manages the lifecycle of orders.

*   `void send_order(const StrategyOrder& order)`
    *   Assigns a Client Order ID (`ClOrdID`), tracks internal state, and forwards to the router.
*   `void on_execution_report(const ExecutionReport& report)`
    *   Updates order state (Filled, Partial, Canceled) based on exchange feedback.

### `ultra::execution::SmartOrderRouter`
Located in `include/ultra/execution/router/sor.hpp`.
Determines the optimal venue for execution.

*   `void route(const OrderState& order)`
    *   Selects destination based on liquidity/fee logic and dispatches to the appropriate gateway.

---

## 3. Market Data

### `ultra::md::OrderBookL2`
Located in `include/ultra/market-data/book/order_book_l2.hpp`.
High-performance L2 Order Book.

*   `void update(const DecodedMessage& msg)`
    *   Applies Add/Delete/Replace updates.
*   `const Level& best_bid() const`
    *   Returns the top-of-book Bid level (Price, Quantity).
*   `const Level& best_ask() const`
    *   Returns the top-of-book Ask level.

### `ultra::md::itch::ITCHDecoder`
Located in `include/ultra/market-data/itch/decoder.hpp`.

*   `DecodedMessage decode(const uint8_t* data, size_t len, Timestamp ts)`
    *   Parses raw bytes into a structured message. Optimized with SIMD.

---

## 4. Telemetry & Monitoring

### `ultra::telemetry::LatencyHistogram`
Located in `include/ultra/telemetry/latency/histogram.hpp`.
Lock-free statistical tracking.

*   `void record(uint64_t latency_ns)`
    *   Buckets a latency sample (relaxed atomic operation).
*   `void print_stats() const`
    *   Outputs P50/P99/Avg stats to stdout.

### `ultra::telemetry::MetricsPublisher`
Located in `include/ultra/telemetry/monitoring/publisher.hpp`.

*   `void start()`
    *   Launches a background thread to snapshot system metrics (1Hz).

---

## 5. Hardware & Network

### `ultra::fpga::FPGADriver`
Located in `include/ultra/fpga/fpga_driver.hpp`.
Controls the hardware offload engine.

*   `bool init()`
    *   Maps the PCIe BAR0 region or allocates simulated memory.
*   `void update_strategy_params(double skew, double risk_aversion, Quantity max_pos)`
    *   Writes high-level parameters to the FPGA registers.

### `ultra::network::MulticastReceiver`
Located in `include/ultra/network/multicast_receiver.hpp`.

*   `int receive(uint8_t* buffer, size_t max_len)`
    *   Reads a packet from the kernel socket ring buffer. Non-blocking.
