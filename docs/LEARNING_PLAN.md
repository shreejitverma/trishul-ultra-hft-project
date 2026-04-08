# Ultra-HFT Engineering Curriculum: From Silicon to Strategy (v0.4.0)

This document outlines a rigorous, **12-week intensive learning path** designed to transform a competent C++ developer into an expert Systems Engineer capable of building the Trishul Ultra-HFT platform.

**Prerequisites:**
*   Intermediate C++ (RAII, STL, basic threading).
*   Basic Linux CLI proficiency.
*   Understanding of digital logic (AND/OR gates, clock cycles) helps but is not mandatory.

---

## Phase 1: Foundations of Low-Latency Systems (Weeks 1-3)

**Goal:** Understand how the machine executes code and how to stop the OS from interfering.

### Week 1: Modern C++ & Memory Model
*   **Concepts:** Zero-overhead abstractions, `constexpr`, Memory Ordering (`acquire`/`release`), Cache Coherency (MESI protocol).
*   **Mini-Project:** Implement a thread-safe `SpinLock` using `std::atomic_flag` and benchmark it against `std::mutex`.

### Week 2: Computer Architecture & Optimization
*   **Concepts:** CPU Pipelines, Branch Prediction, Cache Lines, False Sharing, SIMD (AVX2).
*   **Mini-Project:** Optimize a matrix multiplication function using AVX2 intrinsics (`immintrin.h`) to beat `-O3` auto-vectorization.

### Week 3: Kernel Bypass & Networking
*   **Concepts:** User-space Networking, Ring Buffers, Interrupts vs. Polling, Kernel Bypass (DPDK/Solarflare).
*   **Mini-Project:** Write a UDP Multicast Receiver using `SO_REUSEPORT` and `busy-polling` (non-blocking `recv`) to measure packet inter-arrival times.

---

## Phase 2: Hardware Acceleration (FPGA) (Weeks 4-6)

**Goal:** Learn to think in parallel and design digital circuits for the data path.

### Week 4: Verilog & Digital Logic
*   **Concepts:** combinational vs. sequential logic, `always` blocks, `blocking` (=) vs `non-blocking` (<=) assignments, Reset logic.
*   **Mini-Project:** Implement a reliable UART Transmitter in Verilog and simulate it with Verilator.

### Week 5: AXI Stream & Pipelining
*   **Concepts:** Valid/Ready Handshake, Backpressure, FIFOs, Pipeline Stages for Timing Closure.
*   **Mini-Project:** Build a 3-stage pipelined "Moving Average" filter that accepts AXI-Stream input and produces AXI-Stream output.

### Week 6: Hybrid System Design (PCIe)
*   **Concepts:** Memory Mapped IO (MMIO), DMA, PCIe BARs, Driver Development.
*   **Mini-Project:** Write a simple C++ user-space driver to read/write 32-bit registers from a simulated file representing PCIe BAR0.

---

## Phase 3: Market Microstructure & Strategy (Weeks 7-9)

**Goal:** Understand the financial domain and the math behind the money.

### Week 7: The Limit Order Book (LOB)
*   **Concepts:** Bids/Asks, Spread, Depth, Price-Time Priority, Matching Algorithms.
*   **Mini-Project:** Implement a `MatchingEngine` class in C++ that supports `Limit` and `Market` orders with strict Price-Time priority.

### Week 8: Market Data Protocols (ITCH/OUCH)
*   **Concepts:** Binary Protocols, Little-Endian vs Big-Endian, Delta Encoding, Sequence Numbers.
*   **Mini-Project:** Write a parser that reads a binary file of ITCH messages and reconstructs the Order Book state.

### Week 9: Microstructure & Point Processes
*   **Concepts:** Poisson vs. **Hawkes Processes**, Self-Excitation, Volatility Clustering, **Ogata's Thinning Algorithm**.
*   **Mini-Project:** Implement a Hawkes generator where the arrival of one trade increases the probability of the next; visualize the clusters.

---

## Phase 4: Integration & Advanced Topics (Weeks 10-12)

**Goal:** Bring it all together into the Trishul platform.

### Week 10: Lock-Free Programming Deep Dive
*   **Concepts:** SPSC/MPMC Queues, Ring Buffers, Memory Barriers (`std::memory_order`).
*   **Mini-Project:** Implement the `AsyncLogger` pattern using a custom Ring Buffer and background thread.

### Week 11: Reinforcement Learning for Trading
*   **Concepts:** MDP, PPO, State Space Design, Reward Engineering, **Quantization-Aware Training (QAT)**.
*   **Mini-Project:** Train a PPO agent in Python/Gymnasium to manage inventory risk during a Hawkes-simulated price burst.

### Week 12: Systemic Boundaries & Scaling
*   **Concepts:** **PCIe/DMA Saturation**, Burst Buffers, Scaling Limits, Latency Explosion beyond the "Slowdown Point."
*   **Mini-Project:** Execute the `throughput_stress_test` and identify the transaction-per-second (TPS) limit of your host interconnect.

---

## Essential Tools (2026 Standards)

| Category | Tool | Description |
| :--- | :--- | :--- |
| **OS** | `Ubuntu 24.04` | Latest LTS with modern kernel isolation support. |
| **Build** | `CMake` | Industry standard build system. |
| **Profiling** | `perf` | Linux profiler (cache misses, cycles). |
| **FPGA Sim** | `Verilator` | Fastest open-source Verilog simulator. |
| **Latency** | `hdrhistogram` | High Dynamic Range Histogram for P99 tracking. |
