# Trishul: An AI-Driven Ultra-Low-Latency Market Making and Execution Platform
![Version](https://img.shields.io/badge/version-0.4.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)

**Fast. Sharp. Lethal.**

Trishul is a production-grade, ultra-low-latency C++ system for high-frequency trading (HFT). It serves as the software implementation of the **Hybrid Control Plane** described in the Master's thesis **"AI-Integrated FPGA for Market Making in Volatile Environments."**

This platform bridges the gap between research and live trading, featuring a **Hawkes-Process Market Simulator**, **OUCH 5.0 Direct Market Access**, **FPGA-ready Reinforcement Learning**, and a **Kernel-Bypass Network Simulation**.

---

## Key Projects
*   [**Ultra-Low Latency Engine**](https://github.com/shreejitverma/trishul-ultra-hft-project/tree/main/apps/live-engine): The core C++20 trading system featuring asynchronous logging and sub-microsecond tick-to-trade latency.
*   [**FPGA Hardware RTL**](https://github.com/shreejitverma/trishul-ultra-hft-project/tree/main/fpga/rtl): Verilog implementations of the market data path, including ITCH decoders and single-cycle BBO order books.
*   [**Reinforcement Learning Trainer**](https://github.com/shreejitverma/trishul-ultra-hft-project/tree/main/tools/rl_trainer): A Python-based training suite for developing volatility-aware market-making agents using PPO.
*   [**Market Data Generator**](https://github.com/shreejitverma/trishul-ultra-hft-project/tree/main/tools/data-generator): High-fidelity simulation tool implementing **Ogata's Thinning Algorithm** for self-exciting Hawkes Process arrivals.

## Research
*   [**Master's Thesis (Full PDF)**](https://github.com/shreejitverma/trishul-ultra-hft-project/blob/main/FE900_Thesis/SV_stevensThesis.pdf): AI-Integrated FPGA for Market Making in Volatile Environments (Stevens Institute of Technology). Now 100% complete with 135 pages of 'Godhood' level detail.
*   [**Financial Glossary**](https://github.com/shreejitverma/trishul-ultra-hft-project/blob/main/FE900_Thesis/glossary.tex): Detailed definitions of 30+ HFT and market microstructure terms.
*   [**Architectural Diagrams**](https://github.com/shreejitverma/trishul-ultra-hft-project/tree/main/FE900_Thesis/diagrams): Optimized landscape-mode visual flow of the hardware-software hybrid control plane.

---

## Key Features

### Ultra-Low Latency Core
*   **Zero-Allocation Hot Path:** Object Pools and Ring Buffers prevent runtime heap allocation.
*   **Lock-Free Concurrency:** SPSC Queues for thread-safe, contention-free communication.
*   **Kernel Bypass Foundation:** Simulated `DMARingBuffer` mimics VFIO/DPDK memory management.
*   **Mechanical Sympathy:** AVX2-vectorized signal generation and cache-line padding to prevent false sharing.

### Microstructure & AI
*   **Hawkes Process Simulation:** Captures self-exciting order flow clustering common in high-frequency regimes.
*   **RL-Adaptive Strategies:** PPO-trained policies that proactively mitigate **Adverse Selection Cost ($\mathcal{C}_{as}$)**.
*   **Order Book Imbalance (OBI):** High-frequency signal integration for dynamic quote skewing.

### Hardware-Software Co-Design
*   **Deterministic Execution:** FPGA-based execution path verified at **< 200 ns** tick-to-trade.
*   **Hybrid Control Plane:** PCIe Gen4 x16 interconnect for hot-swapping model weights and zero-copy telemetry.
*   **Pre-Trade Compliance:** Hardware-level enforcement of **SEC Rule 15c3-5** at wire-speed.

---

## Performance Benchmarks (100 Million Event Session)

| Component | Metric | Result |
|-----------|--------|--------|
| **Latency** | Median Tick-to-Trade (T2T) | **822 ns** (Hawkes Load) |
| **Tail Latency** | P99 T2T | **1,180 ns** |
| **Peak Throughput** | Saturation Threshold | **7.5 M msgs/sec** |
| **Breaking Point** | PCIe/DMA Backpressure | **8.5 M msgs/sec** (Latency Surge) |
| **Strategy** | Signal Computation | **< 150 ms** (10M ticks, AVX2) |

---

## Build and Execution

### Security Hardening
The environment uses **Ubuntu 24.04 (LTS)** with pinned, audited dependencies:
*   **GoogleTest:** v1.15.2
*   **PyTorch:** v2.3.1
*   **Numpy/Pandas:** v1.26.4 / v2.2.2

### 1. Build using Docker
```bash
docker build -t ultra-hft-env .
docker run -it --rm -v "$(pwd):/home/builder/project" ultra-hft-env
```

### 2. Compile and Benchmark
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./throughput_stress_test  # Empirical verification of systemic limits
```

---

## References

1.  **Thesis:** "AI-Integrated FPGA for Market Making in Volatile Environments." (2026).
2.  **Hawkes Process:** Ogata, Y. (1981). *On Lewis' simulation method for point processes*.
3.  **Market Making:** Avellaneda, M., & Stoikov, S. (2008). *High-frequency trading in a limit order book*.
4.  **Protocol:** NASDAQ OUCH 5.0 Specification.
