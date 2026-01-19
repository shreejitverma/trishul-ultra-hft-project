# Ultra-HFT Engineering Curriculum: From Silicon to Strategy

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
*   **Resources:**
    *   **Book:** *Effective Modern C++* by Scott Meyers (Chapters 1, 3, 5).
    *   **Book:** *C++ Concurrency in Action* by Anthony Williams (Chapter 5: The C++ memory model).
    *   **Lecture:** [CppCon 2017: Fedor Pikus "C++ Atomicity and Memory Ordering"](https://www.youtube.com/watch?v=9hJkWwHDDxs).
    *   **Lecture:** [CppCon 2014: Scott Meyers "Type Deduction and Why You Care"](https://www.youtube.com/watch?v=wQxj20X-tIU).
*   **Mini-Project:** Implement a thread-safe `SpinLock` using `std::atomic_flag` and benchmark it against `std::mutex`.

### Week 2: Computer Architecture & Optimization
*   **Concepts:** CPU Pipelines, Branch Prediction, Cache Lines, False Sharing, SIMD (AVX2).
*   **Resources:**
    *   **Book:** *Computer Systems: A Programmer's Perspective* (CS:APP) by Bryant & O'Hallaron (Chapters 3, 5, 6).
    *   **Paper:** [What Every Programmer Should Know About Memory](https://people.freebsd.org/~drepper/cpumemory.pdf) by Ulrich Drepper (Sections 3 & 6).
    *   **Lecture:** [MIT 6.172: Performance Engineering of Software Systems](https://www.youtube.com/playlist?list=PLUl4u3cNGP63VIBQVWguHrACcl40lwMTe).
    *   **Tool:** [Compiler Explorer (Godbolt)](https://godbolt.org/) - Analyze assembly output.
*   **Mini-Project:** Optimize a matrix multiplication function using AVX2 intrinsics (`immintrin.h`) to beat `-O3` auto-vectorization.

### Week 3: Kernel Bypass & Networking
*   **Concepts:** User-space Networking, Ring Buffers, Interrupts vs. Polling, Kernel Bypass (DPDK/Solarflare).
*   **Resources:**
    *   **Guide:** [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) (Refresher on Sockets).
    *   **Lecture:** [Kernel Bypass Networking (DPDK)](https://www.youtube.com/watch?v=h2p-Vggj3Qo) - General concept overview.
    *   **Documentation:** [Solarflare Onload User Guide](https://support.solarflare.com/index.php?option=com_cognidox&view=file&id=1566).
*   **Mini-Project:** Write a UDP Multicast Receiver using `SO_REUSEPORT` and `busy-polling` (non-blocking `recv`) to measure packet inter-arrival times.

---

## Phase 2: Hardware Acceleration (FPGA) (Weeks 4-6)

**Goal:** Learn to think in parallel and design digital circuits for the data path.

### Week 4: Verilog & Digital Logic
*   **Concepts:** combinational vs. sequential logic, `always` blocks, `blocking` (=) vs `non-blocking` (<=) assignments, Reset logic.
*   **Resources:**
    *   **Book:** *Digital Design and Computer Architecture* by Harris & Harris.
    *   **Tutorial:** [ZipCPU's Verilog Tutorial](https://zipcpu.com/tutorial/) (Highly recommended for beginners).
    *   **Interactive:** [HDLBits](https://hdlbits.01xz.net/wiki/Main_Page) - Solve the first 50 problems.
*   **Mini-Project:** Implement a reliable UART Transmitter in Verilog and simulate it with Verilator.

### Week 5: AXI Stream & Pipelining
*   **Concepts:** Valid/Ready Handshake, Backpressure, FIFOs, Pipeline Stages for Timing Closure.
*   **Resources:**
    *   **Spec:** [AMBA 4 AXI4-Stream Protocol Specification](https://developer.arm.com/documentation/ihi0051/a).
    *   **Video:** [Xilinx: AXI4-Stream Interface](https://www.youtube.com/watch?v=C2qW8KjF9QY).
*   **Mini-Project:** Build a 3-stage pipelined "Moving Average" filter that accepts AXI-Stream input and produces AXI-Stream output.

### Week 6: Hybrid System Design (PCIe)
*   **Concepts:** Memory Mapped IO (MMIO), DMA, PCIe BARs, Driver Development.
*   **Resources:**
    *   **Guide:** [Xilinx XDMA Driver Guide](https://github.com/Xilinx/dma_ip_drivers).
    *   **Book:** *Linux Device Drivers* (Chapter 9: Communicating with Hardware).
*   **Mini-Project:** Write a simple C++ user-space driver to read/write 32-bit registers from a simulated file representing PCIe BAR0.

---

## Phase 3: Market Microstructure & Strategy (Weeks 7-9)

**Goal:** Understand the financial domain and the math behind the money.

### Week 7: The Limit Order Book (LOB)
*   **Concepts:** Bids/Asks, Spread, Depth, Price-Time Priority, Matching Algorithms.
*   **Resources:**
    *   **Book:** *Trading and Exchanges* by Larry Harris (Chapters 4 & 5).
    *   **Video:** [QuantConnect: Limit Order Books](https://www.youtube.com/watch?v=b1xl-7J2ySQ).
*   **Mini-Project:** Implement a `MatchingEngine` class in C++ that supports `Limit` and `Market` orders with strict Price-Time priority.

### Week 8: Market Data Protocols (ITCH/OUCH)
*   **Concepts:** Binary Protocols, Little-Endian vs Big-Endian, Delta Encoding, Sequence Numbers.
*   **Resources:**
    *   **Spec:** [NASDAQ ITCH 5.0](http://www.nasdaqtrader.com/content/technicalsupport/specifications/dataproducts/NQTVITCHspecification.pdf).
    *   **Spec:** [NASDAQ OUCH 5.0](http://www.nasdaqtrader.com/content/technicalsupport/specifications/TradingProducts/ouch5.0.pdf).
*   **Mini-Project:** Write a parser that reads a binary file of ITCH messages and reconstructs the Order Book state.

### Week 9: Quant Strategy & Risk
*   **Concepts:** Market Making, Inventory Risk, Adverse Selection, Alpha Signals (OBI).
*   **Resources:**
    *   **Paper:** [High-frequency trading in a limit order book (Avellaneda & Stoikov)](https://people.math.carleton.ca/~zhishang/Math%204907/2008%20Avellaneda%20Stoikov.pdf).
    *   **Book:** *Algorithmic Trading & DMA* by Barry Johnson.
*   **Mini-Project:** Implement a simulation where a strategy places quotes around a random walk price; measure PnL and Max Drawdown.

---

## Phase 4: Integration & Advanced Topics (Weeks 10-12)

**Goal:** Bring it all together into the Trishul platform.

### Week 10: Lock-Free Programming Deep Dive
*   **Concepts:** SPSC/MPMC Queues, Ring Buffers, ABA Problem, Memory Reclamation.
*   **Resources:**
    *   **Blog:** [1024cores](https://www.1024cores.net/) - Detailed breakdown of lock-free algos.
    *   **Lecture:** [CppCon 2017: Carl Cook "Lock-free Programming for High Performance"](https://www.youtube.com/watch?v=2eJ5vFO7k_k).
*   **Mini-Project:** Implement the `AsyncLogger` pattern using a custom Ring Buffer and background thread.

### Week 11: Reinforcement Learning for Trading
*   **Concepts:** MDP, Q-Learning, PPO, State Space Design, Reward Engineering.
*   **Resources:**
    *   **Book:** *Reinforcement Learning: An Introduction* by Sutton & Barto.
    *   **Course:** [Hugging Face Deep RL Course](https://huggingface.co/learn/deep-rl-course/unit0/introduction).
*   **Mini-Project:** Use Python/Gymnasium to train a simple agent to "Buy Low, Sell High" on sine-wave data.

### Week 12: Profiling & Optimization
*   **Concepts:** Latency Histograms, Percentiles (P99, P99.9), Kernel Tuning (`isolcpus`).
*   **Resources:**
    *   **Tools:** `perf`, `Google Benchmark`.
    *   **Lecture:** [Chandler Carruth: "Efficiency with Algorithms, Performance with Data Structures"](https://www.youtube.com/watch?v=fHNmRkzxHWs).
*   **Mini-Project:** Profile the full `live_engine` using `perf record` and generate a FlameGraph to identify bottlenecks.

---

## Essential Tools Cheat Sheet

| Category | Tool | Description |
| :--- | :--- | :--- |
| **Build** | `CMake` | Industry standard build system. |
| **Compiler** | `GCC 10+` / `Clang 12+` | Need C++20 support (`-std=c++20`). |
| **Profiling** | `perf` | Linux profiler (cache misses, cycles). |
| **Profiling** | `Hotspot` | GUI for `perf` data. |
| **FPGA Sim** | `Verilator` | Fastest open-source Verilog simulator. |
| **Waveforms** | `GTKWave` | Viewer for Verilog simulation dumps (`.vcd`). |
| **Latency** | `hdrhistogram` | High Dynamic Range Histogram for P99 tracking. |