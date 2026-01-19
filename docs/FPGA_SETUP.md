# FPGA Setup Guide: Hybrid High-Frequency Trading

This guide details how to set up the hardware acceleration path for the **Trishul Ultra-HFT Platform**.
The goal is to move the **Strategy Inference** and **Risk Checks** from the C++ Software path to an FPGA, communicating via PCIe.

---

## 1. Hardware Selection (Under $600 Budget)

For an ultra-low-latency HFT lab setup under $600, we recommend **Xilinx Artix-7** based boards with **PCIe Gen2/Gen3** connectivity. These chips are supported by the **free Vivado WebPACK** license.

### Recommended Board: **QMTECH Artix-7 XC7A100T PCIe**
*   **Chip:** Xilinx Artix-7 XC7A100T-2FGG484I
*   **Interface:** PCIe Gen2 x4
*   **Memory:** DDR3 256MB/512MB
*   **Approx. Cost:** $120 - $160
*   **Availability:** AliExpress / Amazon / eBay

### Alternative: **RHS Research Nitefury II (M.2 Form Factor)**
*   **Chip:** Xilinx Artix-7 XC7A200T
*   **Interface:** PCIe Gen2 x4 (M.2 Key M)
*   **Form Factor:** Fits in a standard NVMe slot (Laptop/Desktop)
*   **Approx. Cost:** ~$250
*   **Note:** Requires custom cooling in some setups.

---

## 2. Software Prerequisites

### A. Xilinx Vivado Design Suite (WebPACK Edition)
1.  Download **Vivado ML Standard Edition** (Free) from [AMD/Xilinx](https://www.xilinx.com/support/download.html).
2.  Select **Artix-7** device support during installation.
3.  Install into `/tools/Xilinx`.

### B. Xilinx XDMA Drivers
We use the Xilinx DMA (XDMA) IP core for high-performance host-to-card communication.
```bash
git clone https://github.com/Xilinx/dma_ip_drivers.git
cd dma_ip_drivers/XDMA/linux-kernel/
sudo make install
sudo modprobe xdma
```

---

## 3. RL Core Integration (`strat_decide.v`)

The repository includes a hardware implementation of the Reinforcement Learning policy in `fpga/rtl/strategy/strat_decide.v`. This module implements a 4-stage pipeline:

1.  **Feature Extraction:** Computes Spread, Inventory Imbalance, and Volatility.
2.  **DSP MAC:** Uses DSP48 slices to compute dot-products of features against stored weights.
3.  **Activation:** Implements a hardware-efficient ReLU.
4.  **Decision:** Comparator logic to trigger Buy/Sell signals.

### Integration Steps
1.  Add `fpga/rtl/strategy/strat_decide.v` to your Vivado project sources.
2.  Connect the **AXI-Stream** interface from the XDMA (Host) to the `strat_decide` input registers.
3.  Map the output signals (`buy`, `sell`) to the **OUCH Encoder** module (also in `fpga/rtl`).

---

## 4. Step-by-Step Setup Instruction

### Step 1: Hardware Installation
1.  Power off the Host PC.
2.  Insert the FPGA board into the PCIe slot (or M.2 slot).
    *   *Warning:* Ensure PCIe power cables are connected if the board requires external power (6-pin).
3.  Connect the JTAG Programmer to the board and the PC's USB port.
4.  Power on the PC.

### Step 2: Build the FPGA Bitstream (Hardware Persona)
1.  Open Vivado.
2.  Create a project for your part (e.g., `xc7a100tfgg484-2`).
3.  Add the **XDMA (DMA/Bridge Subsystem for PCI Express)** IP.
    *   Config: PCIe Gen2 x4, AXI4-Lite Master Interface (for Registers), AXI4-Stream (for Data).
4.  Instantiate `strat_decide.v` in your top-level wrapper.
5.  Map the AXI4-Lite interface to **BAR0** (Base Address Register) for parameter updates.
6.  Generate Bitstream (`.bit`).
7.  Open **Hardware Manager** in Vivado and Program Device.

### Step 3: Verify PCIe Link
Once programmed, restart the PC (warm reboot) so the BIOS enumerates the PCIe device.
```bash
lspci -vd 10ee:
```
You should see a Xilinx device. The XDMA driver should auto-load:
```bash
ls /dev/xdma*
# Should see: /dev/xdma0_user, /dev/xdma0_h2c_0, /dev/xdma0_c2h_0
```

### Step 4: Connect Software Driver
Update the C++ `FPGADriver` to map the real hardware device instead of the simulation vector.

**File:** `src/fpga/driver/fpga_driver.cpp`

**Change `init()` to:**
```cpp
#include <fcntl.h>
#include <sys/mman.h>

bool FPGADriver::init() {
    int fd = open("/dev/xdma0_user", O_RDWR); // BAR0 Access
    if (fd < 0) {
        perror("Failed to open FPGA BAR0");
        return false;
    }
    
    // Map 4KB of Control Registers
    void* map_ptr = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map_ptr == MAP_FAILED) {
        perror("mmap failed");
        return false;
    }
    
    regs_ = reinterpret_cast<ControlRegisters*>(map_ptr);
    return true;
}
```

### Step 5: Run the Engine
```bash
sudo ./apps/live-engine/live_engine
```
The application will now write Strategy Parameters directly to the FPGA's AXI-Lite registers via PCIe.

---

## Troubleshooting

*   **PC doesn't see FPGA:** Try a warm reboot after programming via JTAG. PCIe devices mostly enumerate only at boot.
*   **Permission Denied:** Use `sudo` or add your user to `dialout`/`root` groups for `/dev/xdma*` access.
*   **Latency Spikes:** Ensure `isolcpus` is set in your Linux kernel boot args to isolate the cores used by `live_engine`.