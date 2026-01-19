# FPGA Hardware Pipeline

This directory contains the SystemVerilog/Verilog source code for the FPGA acceleration modules described in the thesis.

## Architecture
The design follows a streaming architecture (AXI-Stream) for low-latency processing:
1.  **Parsers:** `rtl/parsers/rx_parser.v` (Ethernet/IP/UDP Extraction)
2.  **Market Data:** `rtl/parsers/itch_decoder.v` (ITCH 5.0 Message Parsing)
3.  **Order Book:** `rtl/book/book2.v` (L1/BBO Tracking)
4.  **Strategy:** `rtl/strategy/strat_decide.v` (Threshold-based Decision Engine)
5.  **Egress:** `rtl/encoder/order_encode.v` (Binary Order Formatting)

## Simulation & Testing

### Prerequisites
*   **Icarus Verilog** (`iverilog`) OR **Verilator**
*   **GTKWave** (for waveform viewing)

### Running the Network Parser Test
```bash
cd fpga
iverilog -o test_parser rtl/parsers/rx_parser.v tb/tb_rx_parser.v
vvp test_parser
```

### Running the ITCH Decoder Test
```bash
cd fpga
iverilog -o test_itch rtl/parsers/itch_decoder.v tb/tb_itch_decoder.v
vvp test_itch
```

### Running the Order Book Test
```bash
cd fpga
iverilog -o test_book rtl/book/book2.v tb/tb_book2.v
vvp test_book
```

### Running the Strategy Engine Test
```bash
cd fpga
iverilog -o test_strat rtl/strategy/strat_decide.v tb/tb_strat_decide.v
vvp test_strat
```

### Running the Encoder/Egress Test
```bash
cd fpga
iverilog -o test_encoder rtl/encoder/order_encode.v rtl/encoder/tx_bridge.v tb/tb_encoder_egress.v
vvp test_encoder
```

**Expected Output:**
```text
PASS: Order Header Correct (BUY)
PASS: Order Payload Correct (Price)
```
