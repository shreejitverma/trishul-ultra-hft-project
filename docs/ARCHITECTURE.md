# Architecture Overview

**Trishul Ultra-HFT** is a hybrid hardware/software trading platform designed for microsecond-latency market making. It implements the **"Hybrid Control Plane"** architecture, offloading critical path execution to FPGA while managing complex strategy logic in C++.

---

## 1. System Design: The Hybrid Control Plane

The system is divided into two domains:
1.  **Software Domain (C++):** Handles Strategy Logic (RL/Signals), Risk Management, and Parameter Optimization.
2.  **Hardware Domain (FPGA):** Handles Network I/O, Packet Parsing, Book Building, and "Fast Path" Execution.

### Software Architecture (The Event Pipeline)

The C++ engine uses a **Thread-Pinned, Lock-Free Pipeline** pattern to maximize throughput and minimize jitter.

```mermaid
graph TD
    subgraph "Core 1: Market Data"
        Net[Hawkes Simulator] -->|Ogata Thinning| Decoder[ITCH Decoder]
        Decoder -->|Update| Book[L2 Order Book]
        Book -->|BBO Change| Diff[Diff Generator]
        Diff -->|SPSC Queue| Strategy
    end

    subgraph "Core 2: Strategy"
        Strategy{Market Maker Strategy}
        Signal[Signal Engine (AVX2)] -->|OBI / VPIN| Strategy
        Strategy -->|Quote| Risk[Risk Gateway]
        Risk -->|Valid Order| ExecQueue(SPSC Queue)
    end

    subgraph "Core 3: Execution"
        ExecQueue -->|OUCH 5.0| OUCH[OUCH Codec]
        OUCH -->|PCIe DMA| FPGA[FPGA Execution Core]
        FPGA -->|Fill Report| Strategy
    end
    
    subgraph "Background Threads"
        Logger[Async Logger]
        Metrics[Metrics Collector]
    end
```

---

## 2. Key Components

### A. Hawkes Market Simulator
*   **Stochastic Core:** Implements a self-exciting point process where event arrivals trigger further activity, accurately modeling volatility clusters.
*   **Ogata's Thinning:** Uses high-fidelity sampling to ensure synthetic data adheres to market microstructure standards.

### B. Strategy & Signals
*   **Reinforcement Learning (PPO):** Adaptive quoting policy that adjusts spreads based on rolling volatility ($\sigma_t$) and Order Book Imbalance ($\rho_t$).
*   **Vectorized Signal Engine:** Uses **AVX2 SIMD** intrinsics to compute indicators on batches of price data in parallel, delivering sub-150ms processing for 10M events.

### C. Systemic Saturation Management
*   **Backpressure Handling:** The architecture is designed to handle up to **7.5 million events per second** with nanosecond determinism.
*   **DMA Descriptor Optimization:** Minimal descriptor overhead to maximize PCIe Gen4 throughput before reaching the physical transaction limits.

### D. Core Infrastructure
*   **Mechanical Sympathy:** Aligning data structures to cache lines (64 bytes) and using huge pages (2MB/1GB) to eliminate TLB misses.
*   **Lock-Free Sync:** Synchronization via C++11 memory barriers (`memory_order_release/acquire`) instead of mutexes.

---

## 3. FPGA Integration (Hardware Path)

The `fpga/` directory contains the Verilog RTL for the hardware components described in the thesis.

*   **RL Inference Core:** A fully unrolled systolic array of DSP48 slices executing the neural network in **13.3 ns**.
*   **Compliance Gates:** Hardware-level enforcement of **SEC Rule 15c3-5**, checking credit limits and "fat-finger" errors at wire-speed.

---

## 4. Verification Methodology

*   **Bit-Accurate Synchrony:** Cycle-by-cycle comparison between the RTL implementation and the software "Golden Model."
*   **Stress Testing:** The system is verified against a **100 Million Event session** to identify systemic "Breaking Points" and ensure graceful degradation under extreme load.
