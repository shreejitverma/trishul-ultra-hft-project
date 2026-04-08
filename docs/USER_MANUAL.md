# User Manual (v0.4.0)

Welcome to the **Trishul Ultra-HFT** platform. This manual guides you through building, configuring, and running the system.

## 1. System Requirements

*   **OS:** Linux (Ubuntu 24.04 LTS highly recommended) or macOS (Apple Silicon supported).
*   **Compiler:** GCC 11+ or Clang 14+ (C++20 support required).
*   **Hardware:**
    *   CPU: Modern multi-core processor (AVX2 support required for signal vectorization).
    *   RAM: 8GB minimum (16GB recommended for 100M event sessions).
    *   FPGA (Optional): Xilinx UltraScale+ for hardware offload features.

## 2. Building the Project

The project uses CMake and requires specific pinned dependencies for security.

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Run Tests
To verify the build integrity and security hardening:
```bash
ctest --output-on-failure
```

---

## 3. RL Model Training

The system supports training Reinforcement Learning models using Python and Gymnasium.

### Prerequisites
Install the audited Python packages:
```bash
pip install -r tools/rl_trainer/requirements.txt
```

### Training a Model
```bash
python3 tools/rl_trainer/train.py
```

---

## 4. Running Microstructure Simulations

### Step 1: Generate Hawkes Market Data
Generate a synthetic dataset (100 Million trades) with self-exciting microstructure clustering using the **Hawkes Process**.

```bash
./build/data_generator market_data_100m.bin 100000000
```

### Step 2: Run Vectorized Backtest
Run the backtester against the generated data to evaluate the adaptive RL policy.

```bash
./build/strategy_backtester market_data_100m.bin
```

---

## 5. Benchmarking and System Limits

Trishul provides advanced tools to identify the physical boundaries of your hardware.

### Latency Benchmark
Measures the deterministic path latency.
```bash
./build/latency_bench market_data_100m.bin
```

### Throughput Stress Test (Slowdown Analysis)
Empirically identifies the **Slowdown Point** and **Breaking Point** by simulating PCIe/DMA saturation.
```bash
./build/throughput_stress_test
```

**Expected Result:**
- **Optimal Zone:** < 7.5M msgs/sec
- **Saturation Point:** 7.5M - 8.5M msgs/sec
- **Breaking Point:** > 8.5M msgs/sec (Latency spikes to 400us+)

---

## 6. Running the Live Engine

### Simulation Mode (Default)
```bash
sudo ./build/live_engine
```
*Note: `sudo` is required to enable real-time CPU isolation and thread pinning.*

### Live Mode (UDP Ingestion)
1.  Configure the Multicast IP/Port in `apps/live-engine/engine.cpp`.
2.  Launch with live mode enabled:
```bash
export ULTRA_LIVE_MODE=1
sudo ./build/live_engine
```

---

## 7. FPGA Hardware Integration

1.  Follow the [FPGA Setup Guide](FPGA_SETUP.md).
2.  The engine will attempt to map the FPGA BAR via the `vfio-pci` driver.
3.  If detected, the **compliance gates** (SEC Rule 15c3-5) will be active at wire-speed.
