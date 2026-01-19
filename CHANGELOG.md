# Changelog

All notable changes to the Trishul Ultra-HFT project will be documented here.

## [0.2.0] - 2026-01-19

### Added
- **FPGA Hardware Pipeline (`fpga/rtl`)**: Full Verilog implementation of the Tick-to-Trade path.
    - `rx_parser`: Zero-copy Ethernet/IP/UDP header extraction.
    - `itch_decoder`: Hardware parsing of ITCH 5.0 "Add Order" messages.
    - `book2`: Single-cycle L1 Order Book (Best Bid/Offer).
    - `strat_decide`: Deterministic, threshold-based Strategy Engine.
    - `order_encode`: OUCH-style binary order packet generation.
- **Python RL Environment (`tools/rl_trainer`)**:
    - `MarketMakingEnv`: Gymnasium-compatible environment simulating a Limit Order Book.
    - `train.py`: PPO training script using Stable-Baselines3.
- **Execution System (`src/execution`)**:
    - `OrderManagementSystem`: Core OMS for tracking order state and lifecycle.
    - `SmartOrderRouter`: Baseline SOR for venue routing logic.
- **Telemetry System (`src/telemetry`)**:
    - `LatencyHistogram`: Lock-free histogram for nanosecond-precision latency tracking.
    - `MetricsPublisher`: Background service for system health monitoring.

### Changed
- **Strategy Engine**: Replaced stub logic with `RLPolicyStrategy` utilizing the new OMS/SOR interfaces.
- **Documentation**: Updated Architecture diagrams and component descriptions to match the implementation.

## [0.1.0] - 2026-01-13

### Added
- **Vectorized Backtesting Engine (`apps/strategy-backtester`)**: Supports loading 10M+ trades and computing indicators in <200ms.
- **Signal Engine (`SignalEngine`)**: SIMD-optimized implementation of SMA and RSI using AVX2 (x86) and NEON (ARM64) support.
- **Performance Metrics**: Sharpe Ratio, Sortino Ratio, Max Drawdown, and CAGR calculators.
- **FPGA Driver (`FPGADriver`)**: Simulation of PCIe Memory-Mapped I/O for hardware offload.
- **Multicast Receiver**: Kernel-bypass style UDP socket receiver (`SO_REUSEPORT`).
- **Data Generator**: Enhanced GBM model for realistic 10M trade datasets.
- **Documentation**: Comprehensive Architecture, User Manual, and API Reference.

### Changed
- **Order Book**: Replaced `std::map` with `ObjectPool` and Flat Arrays for O(1) lookups and cache locality.
- **Threading**: Integrated `ThreadUtils` for core pinning.
- **Live Engine**: Added `ULTRA_LIVE_MODE` env var to switch between simulation and live feed.

### Performance
- **Throughput**: Increased from ~1.8M to **37.8M messages/sec**.
- **Latency**: Reduced average book update latency to **~6 ticks** (~6ns).
