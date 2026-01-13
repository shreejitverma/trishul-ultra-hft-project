# API Reference

This document outlines the core C++ interfaces for developers extending the Trishul Ultra-HFT platform.

## 1. Strategy & Signals

### `ultra::strategy::SignalEngine`
Located in `include/ultra/strategy/signal_engine.hpp`.
Provides SIMD-accelerated technical indicators.

*   `static std::vector<double> sma(const std::vector<double>& prices, int window)`
    *   Computes Simple Moving Average.
*   `static std::vector<double> rsi(const std::vector<double>& prices, int period)`
    *   Computes Relative Strength Index using rolling averages.
*   `static std::vector<int> ma_crossover_signal(const std::vector<double>& fast, const std::vector<double>& slow)`
    *   Returns `1` (Buy), `-1` (Sell), or `0` (Hold) based on crossover logic.

### `ultra::strategy::RLPolicyStrategy`
Located in `include/ultra/strategy/rl-inference/rl_policy.hpp`.
Implements the trading logic.

*   `void on_market_data(const DecodedMessage& msg)`
    *   Callback for new ITCH messages. Updates internal book and runs inference.
*   `void run_inference()`
    *   Executes the Avellaneda-Stoikov logic to determine quote skew based on inventory.

---

## 2. Market Data

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

## 3. Hardware & Network

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

---

## 4. Utilities

### `ultra::SIMDUtils`
Located in `include/ultra/core/simd_utils.hpp`.

*   `static void compute_rolling_mean(...)`
    *   AVX2 implementation of sliding window sum.
*   `static double calculate_std_dev(...)`
    *   AVX2 implementation of standard deviation.
