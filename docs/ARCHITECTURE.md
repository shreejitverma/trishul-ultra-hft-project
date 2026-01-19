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
        Net[Network / Simulation] -->|ITCH 5.0| Decoder[ITCH Decoder]
        Decoder -->|Update| Book[L2 Order Book]
        Book -->|BBO Change| Diff[Diff Generator]
        Diff -->|SPSC Queue| Strategy
    end

    subgraph "Core 2: Strategy"
        Strategy{Market Maker Strategy}
        Signal[Signal Engine (AVX2)] -->|OBI / RSI| Strategy
        Strategy -->|Quote| Risk[Risk Gateway]
        Risk -->|Valid Order| ExecQueue(SPSC Queue)
    end

    subgraph "Core 3: Execution"
        ExecQueue -->|OUCH 5.0| OUCH[OUCH Codec]
        OUCH -->|Bytes| Matcher[Matching Engine / Exchange]
        Matcher -->|Fill Report| Strategy
    end
    
    subgraph "Background Threads"
        Logger[Async Logger]
        Metrics[Metrics Collector]
    end
```

---

## 2. Key Components

### A. Market Data Ingestion
*   **ITCH Decoder:** Optimized zero-copy decoder for NASDAQ ITCH 5.0.
*   **L2 Order Book:** Flat-map based order book implementation.
*   **Diff Generator:** A smart notifier that triggers strategy callbacks *only* when the Best Bid/Offer (BBO) changes, reducing noise.

### B. Strategy & Signals
*   **Market Maker:** An implementation of the Avellaneda-Stoikov model, enhanced with **Order Book Imbalance (OBI)** signals.
*   **Signal Engine:** Uses **AVX2 SIMD** intrinsics to compute indicators (SMA, RSI, StdDev) on batches of price data in parallel.
*   **Symbol Universe:** O(1) lookup manager for handling multiple symbols with specific metadata (tick size, lot size).

### C. Execution & Protocols
*   **Smart Order Router (SOR):** A hybrid routing engine that inspects the target symbol of every order. High-priority stocks (e.g., AAPL) are routed to the FPGA driver for ultra-low latency execution, while standard assets are handled by the software gateway.
*   **Matching Engine:** A Price-Time Priority simulator that mimics a real exchange. It supports Limit Orders, Aggressive Matching, and Partial Fills.
*   **OUCH 5.0:** The system speaks the native binary protocol of major exchanges (NASDAQ), ensuring the software stack is "Direct Market Access" (DMA) ready.
*   **Execution Reports:** Feedback loop providing Fill Price, Quantity, and Order Status back to the strategy.

### D. Core Infrastructure
*   **Async Logger:** A ring-buffer based logger that serializes messages to a background thread, ensuring zero-latency penalties during trading.
*   **Memory Model:**
    *   **Lock-Free Queues:** SPSC queues for inter-thread communication.
    *   **Object Pools:** Pre-allocated memory for orders to avoid `new`/`delete`.
    *   **Prefaulting:** `mlockall` and stack warmup to prevent Page Faults.
*   **Thread Isolation:** `SCHED_FIFO` priority and CPU affinity settings to minimize OS scheduler jitter.

---

## 3. FPGA Integration (Hardware Path)

The `fpga/` directory contains the Verilog RTL for the hardware components described in the thesis.

*   **RL Core (`strat_decide.v`):** A 4-stage pipeline (Feature Extraction -> DSP48 MAC -> ReLU -> Comparator) that executes the neural network policy in hardware.
*   **Data Flow:**
    1.  C++ Strategy computes optimal weights/parameters.
    2.  Writes parameters to FPGA via PCIe (simulated driver).
    3.  FPGA uses these parameters to make nanosecond-scale decisions on incoming tick data.

---

## 4. Risk Management

Risk is handled in two layers:
1.  **Software Pre-Trade:** The `PretradeChecker` verifies Max Order Size, Position Limits, and Notional Exposure *before* an order leaves the strategy thread.
2.  **Hardware Gate:** The FPGA contains a logic gate that blocks orders violating hard limits (Max Notional), providing a fail-safe.