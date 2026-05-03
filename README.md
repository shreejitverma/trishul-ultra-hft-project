# Trishul: AI-Integrated FPGA for Market Making in Volatile Environments

![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)
![License](https://img.shields.io/badge/license-MIT-green.svg)
![Build Status](https://img.shields.io/badge/build-passing-brightgreen.svg)
![C++](https://img.shields.io/badge/C++-20-blue.svg)
![Verilog](https://img.shields.io/badge/Verilog-RTL-orange.svg)
![Python](https://img.shields.io/badge/Python-3.10%2B-yellow.svg)

**Fast. Sharp. Lethal.**

Trishul is a production-grade, ultra-low-latency (ULL) platform for high-frequency trading (HFT) and market making. It serves as the complete hardware-software implementation of the **Hybrid Control Plane** detailed in the Master's thesis: *"AI-Integrated FPGA for Market Making in Volatile Environments."*

This repository bridges the gap between deep reinforcement learning research and deterministic, wire-speed live trading. It features a custom **Hawkes-Merton Synthetic Market Simulator**, **FPGA-ready PPO Reinforcement Learning**, **Hardware-Accelerated Risk Management (SEC Rule 15c3-5)**, and a **Kernel-Bypass C++ Software Stack**.

---

## 📖 Table of Contents
1. [Abstract \& Overview](#-abstract--overview)
2. [System Architecture](#-system-architecture)
3. [Repository Structure](#-repository-structure)
4. [Core Subsystems](#-core-subsystems)
    - [1. Synthetic Data Engine](#1-synthetic-data-engine-hawkes-merton-hybrid)
    - [2. Ultra-Low-Latency Software Core](#2-ultra-low-latency-software-core)
    - [3. FPGA RTL Components](#3-fpga-rtl-components)
    - [4. Reinforcement Learning \& Analytics](#4-reinforcement-learning--analytics)
5. [Performance Benchmarks](#-performance-benchmarks)
6. [Getting Started](#-getting-started)
7. [Thesis \& Documentation](#-thesis--documentation)
8. [Disclaimer](#-disclaimer)

---

## 🌟 Abstract & Overview

Modern electronic markets are non-stationary and highly toxic, frequently characterized by sub-millisecond liquidity droughts and flash crashes. Traditional quantitative trading systems face a dilemma: **Software-based AI models** are adaptable but suffer from non-deterministic operating system jitter, while **Hardware-based (FPGA) strategies** execute at nanosecond speeds but rely on rigid, simplistic rules.

**Trishul solves this by unifying both paradigms.** 
It leverages Proximal Policy Optimization (PPO) to train volatility-aware market-making agents that are then synthesized into a fixed-point systolic array on an FPGA. This enables complex, adaptive decision-making at absolute wire-speed (**< 900 ns** tick-to-trade latency), entirely bypassing the CPU hot path.

---

## 🏗 System Architecture

The system operates via a **Hardware-Software Co-Design** approach:

1.  **Deterministic Execution Layer (FPGA):** Handles the entire critical path. It ingests 10GbE UDP packets, parses NASDAQ ITCH 5.0 messages, updates an L2 Order Book, extracts microstructural features (like Order Book Imbalance), runs the AI inference, performs pre-trade risk checks, and encodes NASDAQ OUCH 5.0 orders—all within a single clock domain.
2.  **Hybrid Control Plane (CPU/C++20):** Manages asynchronous tasks without interrupting the FPGA. It handles model weight hot-swapping via PCIe AXI-Lite, async lock-free telemetry logging, AVX2-vectorized signal generation, and historical backtesting simulation.

---

## 📂 Repository Structure

```text
ultra-hft-project/
├── FE900_Thesis/          # Full LaTeX source for the 170+ page Master's Thesis
│   ├── diagrams/          # High-quality TikZ architectural diagrams (Landscape)
│   └── SV_stevensThesis.pdf # Compiled final draft
├── apps/
│   ├── live-engine/       # C++20 Core Trading Engine & Hybrid Control Plane
│   └── fpga-sim/          # Verilator / Icarus Verilog testbenches for RTL
├── benchmarks/            # Micro-benchmarks for throughput and tail-latency
├── config/                # TOML profiles for engine tuning and risk limits
├── fpga/                  # Hardware Domain
│   └── rtl/               # Verilog source code
│       ├── parsers/       # ITCH 5.0 Decoders
│       ├── book/          # Hardware L2 Order Book
│       ├── strategy/      # Systolic Array for RL Inference
│       ├── risk/          # Wire-speed SEC 15c3-5 Risk Gate
│       └── encoder/       # OUCH 5.0 Binary Encoder
├── src/                   # C++ Libraries
│   ├── core/              # Lock-free SPSC queues, memory pools, nanosecond clocks
│   └── market-data/       # Software fallback parsers and book builders
├── tools/                 # Python Analytics and ML Domain
│   ├── rl_trainer/        # Stable Baselines3 PPO Market Making Environment
│   ├── data-generator/    # Hawkes-Merton Stochastic Market Simulator
│   └── analysis/          # Model selection, AIC/BIC validation, and PnL plotting
└── Dockerfile             # Reproducible build environment (Ubuntu 24.04 LTS)
```

---

## ⚙️ Core Subsystems

### 1. Synthetic Data Engine (Hawkes-Merton Hybrid)
To push the architecture to its limits, we bypass traditional historical backtesting in favor of a generative approach. 
*   **Merton Jump-Diffusion:** Simulates macroscopic price gaps and fat-tailed volatility.
*   **Ogata's Thinning (Hawkes Process):** Simulates micro-level "liquidity cascades" and self-exciting order flow.
*   *Validation:* Includes a dedicated `model_selection.py` script that justifies the hybrid model using Akaike/Bayesian Information Criteria (AIC/BIC) against baseline Geometric Brownian Motion (GBM).

### 2. Ultra-Low-Latency Software Core
For components requiring software flexibility, Trishul uses extreme C++ mechanical sympathy:
*   **Zero-Allocation Hot Path:** Completely eliminates `malloc`/`new` during the trading session using pre-allocated Memory Pools.
*   **SPSC Lock-Free Queues:** Deterministic, mutex-free inter-thread communication using `std::atomic` and explicit memory ordering (`acquire`/`release`).
*   **Kernel Bypass Simulation:** Maps physically contiguous memory (`MAP_HUGETLB`) to simulate DPDK/VFIO network polling.
*   **Async Logger:** Offloads disk I/O to a background thread to prevent latency spikes in the strategy loop.

### 3. FPGA RTL Components
The verifiable Verilog codebase targeting a Kintex UltraScale+ architecture:
*   **`rx_parser`:** Minimal L2/L3/L4 parser stripping Ethernet/IP/UDP headers at line-rate.
*   **`itch_decoder`:** Fixed-offset binary extraction of NASDAQ ITCH messages.
*   **`book_manager`:** $O(1)$ hardware registers for tracking the Best Bid and Offer (BBO).
*   **`strat_decide`:** DSP48-accelerated systolic array computing ReLU neural network activations.
*   **`risk_gate`:** Mandatory, single-cycle pre-trade risk checks (Fat-finger, max notional, duplicate detection).
*   **`order_encode`:** Endian-aware NASDAQ OUCH 5.0 packet formatter.

### 4. Reinforcement Learning & Analytics
*   **PPO Agent:** Uses Proximal Policy Optimization to learn asymmetric quoting behaviors based on Order Book Imbalance (OBI) and Volume-Synchronized Probability of Informed Trading (VPIN).
*   **Quantization-Aware Training (QAT):** Models are pruned and quantized to 8-bit/16-bit fixed-point formats to fit within the strict timing and resource limits of the FPGA DSP slices.

---

## 📊 Performance Benchmarks

Performance was evaluated using an ensemble of Monte Carlo simulations across 100 Million market events. The system demonstrates linear scaling up to physical PCIe limits.

| Metric | Traditional Software Stack | Trishul (AI-Integrated FPGA) | Improvement |
| :--- | :--- | :--- | :--- |
| **Median Latency (P50)** | 50,000 ns | **822 ns** | ~60x Faster |
| **Tail Latency (P99)** | 250,000 ns | **1,180 ns** | ~211x Tighter |
| **Sustained Throughput** | 1.2 M msgs/sec | **7.5 M msgs/sec** | 6.25x Scale |
| **Sortino Ratio ($\mathcal{Z}$)** | 0.32 | **2.10** | Superior Risk-Adj. Alpha |
| **Max Drawdown** | 12.4% | **4.2%** | Enhanced Capital Preservation |

*(See Chapter 10 of the thesis for comprehensive comparative analysis and drawdown curves).*

---

## 🚀 Getting Started

### Prerequisites
*   Docker (Recommended for guaranteed reproducibility)
*   CMake >= 3.20
*   GCC 11+ or Clang 14+
*   Python 3.10+ (with `numpy`, `pandas`, `stable-baselines3`, `scipy`)
*   Verilator & Icarus Verilog (for RTL simulation)

### Installation & Build

**1. Using Docker (Recommended)**
```bash
# Build the isolated Ubuntu 24.04 environment
docker build -t ultra-hft-env .

# Run interactively, mounting the current directory
docker run -it --rm -v "$(pwd):/home/builder/project" ultra-hft-env
```

**2. Compiling the C++ Core**
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**3. Running the Simulations**
```bash
# Run the High-Fidelity Data Generator (10M Events)
./tools/data-generator/build/generate_data 10000000

# Run the Throughput Stress Test
./build/throughput_bench

# Run Python Model Selection Analysis
python3 tools/analysis/model_selection.py
```

---

## 📚 Thesis & Documentation

The complete academic context, mathematical derivations, and architectural rationale are provided in the Master's Thesis document:
*   📄 **[Read the Full Thesis (PDF)](FE900_Thesis/SV_stevensThesis.pdf)** (170+ Pages)
*   📐 **[View the Architectural Diagrams](FE900_Thesis/diagrams/)** (TikZ sources)
*   🔤 **[Financial Engineering Glossary](FE900_Thesis/glossary.tex)** 

If utilizing this work in academic or industrial research, please refer to the thesis for deep dives into stochastic control equations, Markov regime-switching models, and FPGA synthesis timing closure constraints.

---

## ⚖️ Disclaimer

**Educational and Research Purposes Only.**

This software and the accompanying documentation are provided for academic research and demonstrative purposes. The codebase simulates direct market access (DMA) protocols and kernel-bypass techniques. **Do not connect this system to live financial exchanges or use it with real capital.** The authors assume no liability for any financial losses or regulatory violations incurred through the use, misuse, or modification of this code.

---
*Developed by Shreejit Verma.*