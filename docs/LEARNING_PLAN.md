# Extensive Learning Plan: Ultra-HFT Hybrid Execution Project

This document outlines a structured learning path to master the technologies, concepts, and architectures used in the Trishul Ultra-HFT platform. It is designed to take a developer from intermediate C++ proficiency to advanced low-latency systems engineering.

---

## 1. Core Technologies

### Language Fundamentals: Modern C++ (C++20)
*   **Focus:** Zero-overhead abstractions, compile-time computation, and memory management.
*   **Key Concepts:**
    *   `constexpr` / `consteval` for compile-time logic.
    *   Templates and Concepts (`<concepts>`) for type safety without virtual overhead.
    *   Memory Model: `std::atomic`, memory ordering (`acquire`/`release`), and alignment (`alignas`).
    *   Pointers: Raw pointers vs. Smart pointers in hot paths (avoid `shared_ptr`).
*   **Resources:**
    *   📖 *Effective Modern C++* by Scott Meyers.
    *   📖 *C++ Concurrency in Action* by Anthony Williams (Essential for lock-free queues).
    *   🔗 [CppReference](https://en.cppreference.com/) - The bible.
    *   🔗 [Talk: "Writing Good C++14"](https://www.youtube.com/watch?v=1OEu9C51K2A) by Bjarne Stroustrup.

### Hardware & System Architecture
*   **Focus:** Understanding the machine to write sympathetic code.
*   **Key Concepts:**
    *   CPU Caches (L1/L2/L3), Cache Lines, False Sharing.
    *   Branch Prediction and Pipelining.
    *   SIMD (AVX2/AVX-512) for vectorized math.
    *   PCIe Bus architecture (for FPGA communication).
*   **Resources:**
    *   📖 *Computer Systems: A Programmer's Perspective* by Bryant & O'Hallaron.
    *   🔗 [Agner Fog's Optimization Manuals](https://www.agner.org/optimize/).
    *   🔗 [What Every Programmer Should Know About Memory](https://people.freebsd.org/~drepper/cpumemory.pdf) by Ulrich Drepper.

### FPGA & Verilog (Hardware Path)
*   **Focus:** Digital logic design for deterministic latency.
*   **Key Concepts:**
    *   Clock Domains and Crossing (CDC).
    *   Finite State Machines (FSM) for protocol parsing.
    *   AXI4-Stream Protocol for data flow.
    *   DSP Slices for arithmetic (RL inference).
*   **Resources:**
    *   📖 *Digital Design and Computer Architecture* by Harris & Harris.
    *   🔗 [ZipCPU's Verilog Tutorial](https://zipcpu.com/tutorial/).
    *   🔗 [Xilinx UltraScale Architecture User Guide](https://docs.xilinx.com/).

---

## 2. Project-Specific Modules

### Module A: Low-Latency Networking & Protocols
*   **Concepts:** UDP Multicast, TCP/IP Stack Bypass, Binary Protocols.
*   **Project Relevance:** `ITCHDecoder`, `OUCHCodec`, `MulticastReceiver`.
*   **Outcome:** Ability to parse 40Gbps feeds with <100ns latency.
*   **Resources:**
    *   🔗 [NASDAQ ITCH 5.0 Specification](http://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf).
    *   🔗 [NASDAQ OUCH 5.0 Specification](http://www.nasdaqtrader.com/content/technicalsupport/specifications/TradingProducts/ouch5.0.pdf).
    *   🔗 [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/).

### Module B: Lock-Free Concurrency & Memory
*   **Concepts:** SPSC Queues, Ring Buffers, Memory Barriers, Object Pools.
*   **Project Relevance:** `SPSCQueue`, `ObjectPool`, `DMARingBuffer`.
*   **Outcome:** Thread communication with zero mutex contention and zero heap allocation.
*   **Resources:**
    *   🔗 [1024cores - Lockfree Algorithms](https://www.1024cores.net/).
    *   🔗 [Folly Library (Facebook) Source Code](https://github.com/facebook/folly).

### Module C: Market Microstructure & Strategy
*   **Concepts:** Limit Order Books (LOB), Matching Engines, Market Making, Adverse Selection.
*   **Project Relevance:** `OrderBookL2`, `MarketMaker`, `MatchingEngine`.
*   **Outcome:** Understanding how to quote profitably and avoid toxic flow.
*   **Resources:**
    *   📖 *Trading and Exchanges* by Larry Harris.
    *   📖 *Algorithmic Trading & DMA* by Barry Johnson.
    *   📄 [High-frequency trading in a limit order book (Avellaneda & Stoikov)](https://people.math.carleton.ca/~zhishang/Math%204907/2008%20Avellaneda%20Stoikov.pdf).

---

## 3. Development Practices

### Version Control: Git Workflow
*   **Strategy:** Feature branching (`feature/xyz`), atomic commits, descriptive messages.
*   **Tools:** Git, GitHub Actions.
*   **Resource:** [Pro Git Book](https://git-scm.com/book/en/v2).

### Testing: Unit & Integration
*   **Framework:** CTest (CMake), Custom GTest-style macros.
*   **Focus:** Testing deterministic logic (Matching Engine) and side-effects (Router).
*   **Resource:** [Google Test Documentation](https://google.github.io/googletest/).

### Performance Profiling
*   **Tools:** `perf`, `hotspot`, `vtune`, `rdtsc`.
*   **Metrics:** Cache misses per instruction (CPI), Branch mispredictions.
*   **Resource:** [Brendan Gregg's Perf Examples](https://www.brendangregg.com/perf.html).

---

## 4. Deployment & Operations

### Environment Setup
*   **Docker:** Containerized builds for reproducible `x86-64` environments (even on ARM).
*   **Kernel Tuning:** `isolcpus`, `hugepages`, `network latency` tuning (`ethtool`).

### Monitoring
*   **Tools:** InfluxDB (Time-series), Grafana (Visualization).
*   **Metrics:** Tick-to-Trade Latency (ns), Order-to-Ack Latency, Strategy PnL.

---

## 5. Measurable Outcomes

### Proficiency Assessment
1.  **Code Optimization:** Can you rewrite a standard `std::vector` loop to use AVX2 intrinsics?
2.  **Architecture:** Can you draw the data path from NIC to FPGA to CPU and explain every buffer?
3.  **Debugging:** Can you diagnose a race condition in a lock-free queue using TSan?

### Project Contribution Targets
*   **Latency:** Reduce software T2T latency by 10% (current baseline: 850ns).
*   **Features:** Implement a new strategy signal (e.g., VWAP or Kalman Filter).
*   **Reliability:** Achieve 100% test coverage for the `MatchingEngine`.
