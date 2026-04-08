# FPGA Setup Guide: Hybrid High-Frequency Trading

This guide details how to set up the hardware acceleration path for the **Trishul Ultra-HFT Platform**.
The goal is to move the **Strategy Inference**, **Risk Checks**, and **Regulatory Compliance** from the C++ Software path to an FPGA, communicating via PCIe Gen4.

---

## 1. Hardware Selection (Production Performance)

To achieve the **< 200 ns** latency verified in our 100M event benchmarks, we recommend **Xilinx UltraScale+** based boards with **PCIe Gen4 x16** and **10GbE SFP+** connectivity.

### Recommended Board: **Alveo U50 / U250**
*   **Chip:** UltraScale+ (Architecture used in Thesis)
*   **Interface:** PCIe Gen4 x16
*   **Network:** 1x / 2x 100GbE (Rate-limited to 10GbE for ITCH/OUCH)
*   **Approx. Cost:** $1,500 - $3,000 (New) / ~$600 (Used)

### Budget Entry: **QMTECH Artix-7 XC7A100T PCIe**
*   **Note:** Limited to 1GbE/PCIe Gen2. Suitable for initial logic verification but will hit the **Breaking Point** at < 1M events/sec.

---

## 2. Software Prerequisites

### A. Xilinx Vivado Design Suite
1.  Download **Vivado ML Standard Edition** from [AMD/Xilinx](https://www.xilinx.com/support/download.html).
2.  Select **UltraScale+** device support during installation.

### B. Xilinx XDMA / QDMA Drivers
We use the **XDMA** IP core for deterministic host-to-card communication.
```bash
git clone https://github.com/Xilinx/dma_ip_drivers.git
cd dma_ip_drivers/XDMA/linux-kernel/
sudo make install
sudo modprobe xdma
```

---

## 3. Core Logic Integration

### A. RL Inference Systolic Array (`strat_decide.v`)
Add `fpga/rtl/strategy/strat_decide.v` to your project. This module implements the fully unrolled PPO policy using **DSP48** slices.

### B. Compliance Gates (SEC Rule 15c3-5)
The `risk_gate.v` module must be placed in series with the **OUCH Encoder**.
1.  **Credit Limit:** Map `BAR0 + 0x10` to the max notional register.
2.  **Duplicate Detection:** Enables the hardware Bloom Filter to prevent runaway loops.

---

## 4. Physical Deployment & Sync

### Precision Time Protocol (PTP) Setup
To achieve nanosecond-scale timestamping:
1.  Connect your FPGA's SMA clock input to a **Grandmaster Clock** (GPS disciplined).
2.  Instantiate the **PTP Servo Core** in Vivado.
3.  The `FPGADriver` will automatically read the `PTP_OFFSET` register at `BAR0 + 0x40`.

---

## 5. Verification: Bit-Accurate Verification

Prior to live trading, execute the UVM-based verification:
1.  Generate Hawkes data: `./build/data_generator market_data_uvm.bin 1000000`
2.  Run the C++ "Golden Model": `./build/strategy_backtester market_data_uvm.bin`
3.  Load the hardware simulation and verify that the `strat_decide` outputs match the software model exactly (Bit-for-Bit parity).

---

## Troubleshooting

*   **PCIe Link Training Failure:** Check the physical PCIe slot version. UltraScale+ requires Gen4 for the **7.5M msgs/sec** throughput target.
*   **Buffer Overflows:** If you observe "BUFFER SATURATION" in the `throughput_stress_test`, increase the `BURST_BUFFER_DEPTH` parameter in your Vivado project.
*   **SEC Compliance Triggered:** If the hardware "Kill Switch" is active, check the `notional_limit` register value via MMIO.
