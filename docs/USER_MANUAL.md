# User Manual

Welcome to the **Trishul Ultra-HFT** platform. This manual guides you through building, configuring, and running the system.

## 1. System Requirements

*   **OS:** Linux (Ubuntu 20.04/22.04 recommended) or macOS (Apple Silicon supported).
*   **Compiler:** GCC 10+ or Clang 12+ (C++20 support required).
*   **Hardware:**
    *   CPU: Modern multi-core processor (AVX2 support recommended).
    *   RAM: 4GB minimum.
    *   FPGA (Optional): Xilinx Artix-7/UltraScale+ for hardware offload features.

## 2. Building the Project

The project uses CMake.

```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Run Tests
To verify the build integrity:
```bash
ctest --output-on-failure
```

### Build Artifacts
Binaries are typically located in the `build/` directory (or subdirectories if configured):
*   `live_engine`: The main trading application.
*   `strategy_backtester`: The vectorized backtesting engine.
*   `data_generator`: Tool to create synthetic market data.
*   `latency_bench`: Micro-benchmark for hot-path components.
*   `throughput_bench`: Benchmark for system capacity.

---

## 3. RL Model Training (New)

The system supports training Reinforcement Learning models using Python and Gymnasium.

### Prerequisites
Install the required Python packages:
```bash
pip install -r tools/rl_trainer/requirements.txt
```

### Training a Model
To train a PPO agent against the simulated Limit Order Book environment:

```bash
python3 tools/rl_trainer/train.py
```

This will:
1.  Initialize the `MarketMakingEnv`.
2.  Train a PPO policy for 10,000 timesteps.
3.  Save the model to `models/ppo_market_maker.zip`.

*Note: Future versions will support exporting this model to ONNX for C++ inference.*

---

## 4. Running Simulations

### Step 1: Generate Market Data
Before backtesting, you need data. Generate a synthetic dataset (10 Million trades) with realistic volatility (GBM).

```bash
./build/data_generator market_data_large.bin 10000000
```

### Step 2: Run Vectorized Backtest
Run the backtester against the generated data. This will output a performance report (Sharpe, Returns, etc.).

```bash
./build/strategy_backtester market_data_large.bin
```

**Output Example:**
```
=== PERFORMANCE REPORT ===
Total Return  : 12.50%
CAGR          : 12.50%
Sharpe Ratio  : 1.8520
Sortino Ratio : 2.1050
Max Drawdown  : 4.20%
Win Rate      : 55.30%
Total Trades  : 9523813
==========================
```

---

## 5. Running the Live Engine

The `live_engine` can run in **Simulation Mode** (internal data loop) or **Live Mode** (UDP Multicast).

### Simulation Mode (Default)
Simply run the executable. It will simulate market data and order matching internally using the `MatchingEngine`.

```bash
sudo ./build/live_engine
```
*Note: `sudo` is recommended on Linux to enable thread pinning and real-time scheduling.*

### Live Mode (UDP Ingestion)
To connect to a real multicast feed (e.g., a local replay or exchange feed):

1.  Edit `apps/live-engine/engine.cpp` to set your Multicast IP/Port.
2.  Run with the environment variable:

```bash
export ULTRA_LIVE_MODE=1
sudo ./build/live_engine
```

---

## 6. Benchmarking

Validate the system performance on your hardware.

### Latency Benchmark
Measures `RDTSC`, `Decoding`, and `Book Update` latency in nanoseconds/ticks.

```bash
./build/latency_bench
```

### Throughput Benchmark
Measures how many millions of messages per second the system can process.

```bash
./build/throughput_bench market_data_large.bin
```

---

## 7. FPGA Integration

If you have a supported FPGA board:
1.  Follow the [FPGA Setup Guide](FPGA_SETUP.md) to install drivers and program the `strat_decide.v` core.
2.  The `live_engine` will automatically detect the PCIe device (`/dev/xdma0_user`) and switch the driver from simulation to MMIO mode.