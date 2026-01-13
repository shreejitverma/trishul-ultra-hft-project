# Architecture Overview

**Trishul Ultra-HFT** is designed as a hybrid hardware/software trading platform where the critical path (execution and risk) is offloaded to FPGA, while the control plane (strategy logic, complex inference) runs on a highly optimized C++ software stack.

## 1. High-Level Design (Hybrid Control Plane)

The system follows the **"Hybrid Control Plane"** architecture described in the project's thesis.

```mermaid
graph TD
    subgraph "Host CPU (C++ Engine)"
        Strategy[AI Strategy (RLPolicy)]
        RiskControl[Risk Parameters]
        Driver[FPGA PCIe Driver]
    end

    subgraph "FPGA (Hardware)"
        MMIO[AXI-Lite Registers]
        HwRisk[Hardware Risk Engine]
        HwExec[Execution Engine (TCP/IP)]
        HwBook[L2 Order Book]
    end

    Strategy -->|Params (Skew, Gamma)| Driver
    RiskControl -->|Limits (Max Pos)| Driver
    Driver -->|PCIe Write| MMIO
    MMIO --> HwRisk
    MMIO --> HwExec
```

### Key Components

1.  **C++ Control Plane (`apps/live-engine`)**:
    *   Runs the reinforcement learning models (or approximations like Avellaneda-Stoikov).
    *   Adjusts high-level parameters (Inventory Skew, Risk Aversion) based on market regimes.
    *   Communicates with the FPGA via a memory-mapped driver (`src/fpga/driver`).

2.  **FPGA Execution Plane (Simulated by `gateway_sim`)**:
    *   In the real system, this handles the physical network layer (UDP/TCP), parsing, and order generation in nanoseconds.
    *   In this software project, we simulate this interaction using `FPGADriver` and `GatewaySim`.

---

## 2. Software Architecture: The Event Pipeline

The software engine uses a **Thread-Pinned, Lock-Free Pipeline** to minimize latency and jitter.

### Core Threads
The system pins threads to specific CPU cores to prevent cache thrashing:
1.  **MD Thread (Core 1):** Ingests UDP Multicast market data, decodes ITCH 5.0, and updates the Order Book.
2.  **Strategy Thread (Core 2):** Reads the updated book, runs SIMD-optimized signal engines, and updates FPGA parameters.
3.  **Exec Thread (Core 3):** Handles execution reports and synchronous risk checks (for software-only trades).

### Data Flow
```
[Network] -> (UDP Recv) -> [MD Thread] -> (SPSC Queue) -> [Strategy Thread] -> (SPSC Queue) -> [Exec Thread]
                                                                    |
                                                                    v
                                                               [FPGA Driver] -> (PCIe) -> [Hardware]
```

---

## 3. Memory Model & Optimization

### Zero-Allocation Principle
Dynamic memory allocation (`malloc`/`new`) is strictly forbidden in the hot path.
*   **Object Pools:** Order objects are allocated from a fixed-size `ObjectPool` backed by Huge Pages (`2MB`).
*   **Flat Maps:** The `OrderBookL2` uses open-addressing hash maps and flat arrays (`std::array`) instead of node-based trees (`std::map`) to ensure cache locality.

### SIMD Acceleration
The `SignalEngine` uses AVX2 (x86) and NEON (ARM64) intrinsics to vectorize CPU-bound tasks:
*   **Indicators:** SMA, RSI, and StdDev are computed on arrays of data in parallel.
*   **Search:** Finding price levels uses SIMD compare-and-mask operations.

### Kernel Bypass
*   **Networking:** Uses `SO_REUSEPORT` and busy-polling on sockets (simulating DPDK/Solarflare `ef_vi` behavior).
*   **Timing:** Uses `RDTSC` (Time Stamp Counter) for sub-nanosecond precision measurements.

---

## 4. Directory Structure

*   `apps/`: Executables (Live Engine, Backtester).
*   `include/ultra/`: Header-only core libraries.
*   `src/`: Implementation files.
*   `fpga/`: Hardware Description Language (Verilog/VHDL) files.
*   `benchmarks/`: Micro-benchmarks for latency and throughput.
