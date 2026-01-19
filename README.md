# Trishul: An AI-Driven Ultra-Low-Latency Market Making and Execution Platform
![Version](https://img.shields.io/badge/version-0.3.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)

**Fast. Sharp. Lethal.**

Trishul is a production-grade, ultra-low-latency C++ system for high-frequency trading (HFT). It serves as the software implementation of the **Hybrid Control Plane** described in the Master's thesis **"AI-Integrated FPGA for Market Making in Volatile Environments."**

This platform bridges the gap between research and live trading, featuring a **Matching Engine Simulator**, **OUCH 5.0 Direct Market Access**, **FPGA-ready Reinforcement Learning**, and a **Kernel-Bypass Network Simulation**.

---

## Key Features

### Ultra-Low Latency Core
*   **Zero-Allocation Hot Path:** Object Pools and Ring Buffers prevent runtime heap allocation.
*   **Lock-Free Concurrency:** SPSC Queues (`lockfree::SPSCQueue`) for thread-safe, contention-free communication.
*   **Kernel Bypass Foundation:** Simulated `DMARingBuffer` mimics VFIO/DPDK memory management.
*   **Thread Optimization:** Real-time scheduling (`SCHED_FIFO`), core isolation, and memory prefaulting (`mlockall`).
*   **Busy-Spin Waiting:** CPU-optimized spin-loops (`_mm_pause`) for sub-microsecond reaction times.

### Advanced Architecture
*   **Hybrid Execution Routing:** Dynamic `SmartOrderRouter` directs latency-sensitive stocks to FPGA while handling others on CPU.
*   **Matching Engine:** Price-Time Priority simulator with full execution feedback (Fills/Partials).
*   **OUCH 5.0 Protocol:** Binary order entry protocol encoder/decoder (Little Endian/Packed) for DMA.
*   **Asynchronous Logging:** High-throughput, non-blocking logger using dedicated background threads.
*   **Symbol Universe:** O(1) lookup manager for multi-symbol trading environments.

### AI and Quantitative Strategy
*   **RL-Ready:** `RLPolicyStrategy` implements Inventory-Aware Avellaneda-Stoikov logic.
*   **Predictive Signals:** **Order Book Imbalance (OBI)** signal with skewing logic based on liquidity pressure.
*   **Vectorized Indicators:** SIMD-accelerated (AVX2/NEON) SMA, RSI, and Standard Deviation calculations.

### Risk and Compliance
*   **Pre-Trade Risk Gateway:** Inline checks for Max Order Size, Position Limits, and Notional Exposure.
*   **Telemetry:** Centralized `MetricsCollector` aggregating T2T latency, CPU usage, and PnL for InfluxDB.

---

## Documentation

*   [**Architecture Overview**](docs/ARCHITECTURE.md): Deep dive into the Event Pipeline and Memory Model.
*   [**API Reference**](docs/API_REFERENCE.md): Class documentation for Strategies, Gateways, and Core Utils.
*   [**User Manual**](docs/USER_MANUAL.md): Detailed setup, configuration, and running guide.
*   [**FPGA Setup**](docs/FPGA_SETUP.md): Hardware integration guide for the RL Core.
*   [**Examples**](docs/EXAMPLES.md): Code snippets for custom strategies and logging.
*   [**Contributing**](CONTRIBUTING.md): Developer guidelines.

---

## Build and Execution

### Prerequisites
*   **OS:** Linux (x86-64) or macOS (Apple Silicon supported via Docker).
*   **Compiler:** GCC 10+ or Clang 12+ (C++20 required).
*   **Tools:** CMake 3.22+, Make.

### 1. Build using Docker (Recommended)
Ensures a consistent, production-parity environment.

```bash
# Build the container
docker build -t ultra-hft-env .

# Enter the container
docker run -it --rm -v "$(pwd):/home/builder/project" ultra-hft-env
```

### 2. Compile the System

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 3. Run the Test Suite
Verify all components (Matching Engine, OUCH, Strategies).

```bash
ctest --output-on-failure
```

### 4. Launch the Live Engine

```bash
./apps/live-engine/live_engine
```

---

## Performance Benchmarks (Apple M3 Simulation)

| Component | Metric | Result |
|-----------|--------|--------|
| **Latency** | Tick-to-Trade (T2T) | **< 850 ns** (Simulated) |
| **Throughput** | Market Data Ingestion | **37.8 M msgs/sec** |
| **Strategy** | Signal Computation | **< 160 ms** (10M ticks, Vectorized) |
| **Logging** | Write Latency | **Zero** (Async/Non-blocking) |

---

## Project Structure

```
Ultra-HFT
├── apps/                   # Executables (Live Engine, Backtester)
├── include/ultra/          # Header-only Core Libraries
│   ├── core/               # RingBuffers, Allocators, Logger, ThreadUtils
│   ├── market-data/        # ITCH Decoder, OUCH Codec, OrderBook
│   ├── strategy/           # Signal Engine, MarketMaker, RL Policy
│   ├── execution/          # Matching Engine, Execution Reports
│   ├── risk/               # Pre-trade Risk Checkers
│   └── telemetry/          # Metrics Collector
├── src/                    # Implementation Files
├── fpga/                   # Verilog RTL (RL Core, Strat Decide)
└── tests/                  # Unit Tests (GTest style)
```

---

## References

1.  **Thesis:** "AI-Integrated FPGA for Market Making in Volatile Environments." (2024).
2.  **Market Making:** Avellaneda, M., & Stoikov, S. (2008). *High-frequency trading in a limit order book*.
3.  **Architecture:** Lock-free programming techniques (SPSC Queues, Ring Buffers).
4.  **Protocol:** NASDAQ OUCH 5.0 Specification.
