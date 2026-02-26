#pragma once
#include "../core/types.hpp"
#include <vector>
#include <array>
#include <cstdint>
#include "../strategy/strategy.hpp" // For StrategyOrder

namespace ultra::fpga {

/**
 * FPGA Control Plane Driver
 * Simulates communication with the FPGA via PCIe BARs / Shared Memory.
 * 
 * In the Thesis architecture:
 * - The C++ CPU Strategy calculates 'High-Level Parameters' (Risk limits, Base Skew, Volatility).
 * - It writes these parameters to FPGA Registers.
 * - The FPGA uses these params to execute micro-second trades.
 */
class FPGADriver {
public:
    // Simulated Register Map (aligned to 64-bit words)
    struct ControlRegisters {
        uint64_t command;           // 0x00: Start/Stop/Reset
        uint64_t heartbeat;         // 0x08: FPGA Liveness
        
        // Strategy Parameters
        int64_t  base_skew;         // 0x10: Base inventory skew
        uint64_t risk_aversion;     // 0x18: Gamma (scaled)
        uint64_t max_pos_limit;     // 0x20: Position Limit
        uint64_t min_spread;        // 0x28: Min spread width
        
        // Status (Read-Only from FPGA)
        int64_t  fpga_inventory;    // 0x30: Current inventory on FPGA
        uint64_t execution_count;   // 0x38: Total fills
        
        // Order Injection (CPU -> FPGA)
        uint64_t order_inject_trigger; // Write 1 to trigger
        uint64_t order_inject_px; ///< int variable representing order_inject_px.
        uint64_t order_inject_qty; ///< int variable representing order_inject_qty.
        uint64_t order_inject_side; ///< int variable representing order_inject_side.
    };

    /**
     * @brief Auto-generated description for FPGADriver.
     */
    FPGADriver();
    /**
     * @brief Auto-generated description for ~FPGADriver.
     */
    ~FPGADriver();

         /**
          * @brief Auto-generated description for init.
          * @return bool value.
          */
    bool init();
    
    // Write Strategy Parameters to FPGA
    void update_strategy_params(double skew, double risk_aversion, Quantity max_pos);
    
    // Direct Execution (CPU routes order to FPGA for low-latency dispatch)
    void send_order(const strategy::StrategyOrder& order);

    // Read Status from FPGA
    int64_t get_fpga_inventory() const;
    uint64_t get_execution_count() const;
    
    // Direct Register Access (Debug/Low-level)
    void write_reg(size_t offset, uint64_t value);
    uint64_t read_reg(size_t offset) const;

private:
    // Simulated Shared Memory / MMIO Region
    // In production, this would be mmap("/dev/uio0")
    ControlRegisters* regs_{nullptr};  ///< ControlRegisters * variable representing regs_.
    std::vector<uint8_t> simulated_memory_;  ///< int variable representing simulated_memory_.
};

} // namespace ultra::fpga
