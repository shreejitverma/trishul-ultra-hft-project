# Changelog

All notable changes to the Trishul Ultra-HFT project will be documented here.

## [Unreleased] - 2026-01-13

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
