# API Reference (v0.4.0)

This document details the public interfaces of the core C++ components.

## 1. Strategy & Logic

### `ultra::strategy::MarketMaker`
Located in `include/ultra/strategy/market_maker.hpp`.
A generic Liquidity Provision strategy.

*   `MarketMaker(SymbolId symbol_id)`
    *   Initializes the strategy for a specific symbol.
*   `void on_market_data(const DecodedMessage& msg)`
    *   Updates the internal book. Triggers quote generation on BBO change.
*   **Microstructure Features:** Now supports features derived from **Hawkes-distributed** order flow, including **VPIN** proxies and Order Book Imbalance (OBI).

### `ultra::strategy::SignalEngine`
Located in `include/ultra/strategy/signal_engine.hpp`.
Vectorized calculation engine.

*   `static std::vector<double> sma(const std::vector<double>& prices, int window)`
    *   AVX2-optimized Simple Moving Average.
*   `static void batch_calculate_volatility(...)`
    *   SIMD-accelerated rolling standard deviation.

---

## 2. Execution & Protocols

### `ultra::exec::MatchingEngine`
Located in `include/ultra/execution/matching_engine.hpp`.
Simulates an exchange matching logic with Price-Time Priority.

### `ultra::execution::SmartOrderRouter`
Located in `include/ultra/execution/router/sor.hpp`.
Hybrid execution router.

*   `void route(const StrategyOrder& order)`
    *   Dynamic steering based on **Adverse Selection Sensitivity ($\Xi$)**.

---

## 3. Infrastructure & Utils

### `ultra::AsyncLogger`
Located in `include/ultra/core/async_logger.hpp`.
Zero-latency logging utility using **C++11 Memory Barriers**.

### `ultra::ThreadUtils`
Located in `include/ultra/core/thread_utils.hpp`.

*   `static void isolate_thread(int core_id)`
    *   Pins thread to a core and sets `SCHED_FIFO` real-time priority. Now supports **Ubuntu 24.04** kernel isolation structures.

---

## 4. Tools & Benchmarking

### `data_generator` (Hawkes Market Simulator)
Located in `tools/data-generator/main.cpp`.
*   **Algorithm:** Ogata's Thinning Algorithm.
*   **Model:** $\lambda(t) = \lambda_0 + \sum \alpha e^{-\beta(t - t_i)}$.

### `throughput_stress_test`
Located in `benchmarks/throughput/stress_test.cpp`.
*   **Purpose:** Empirical identification of the PCIe/DMA saturation bound.
*   **Metrics:** Average Latency (ns), HW Buffer Fill Status, Backpressure Events.
