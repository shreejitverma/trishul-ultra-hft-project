#include "ultra/fpga/fpga_driver.hpp"
#include <iostream>
#include <cstring>

namespace ultra::fpga {

FPGADriver::FPGADriver() {
    // Allocate simulated MMIO region (4KB page aligned typically)
    simulated_memory_.resize(4096);
    regs_ = reinterpret_cast<ControlRegisters*>(simulated_memory_.data());
}

FPGADriver::~FPGADriver() {
    // Unmap in real driver
}

bool FPGADriver::init() {
    std::cout << "[FPGA Driver] Initializing PCIe Interface..." << std::endl;
    // Reset registers
    memset(regs_, 0, sizeof(ControlRegisters));
    regs_->command = 1; // ENABLE
    regs_->heartbeat = 0xDEADBEEF;
    std::cout << "[FPGA Driver] Interface Active. Heartbeat: 0x" << std::hex << regs_->heartbeat << std::dec << std::endl;
    return true;
}

void FPGADriver::update_strategy_params(double skew, double risk_aversion, Quantity max_pos) {
    // Convert doubles to fixed-point for FPGA
    // Assuming 16.16 fixed point or similar scaling
    const double SCALE = 10000.0;
    
    regs_->base_skew = static_cast<int64_t>(skew * SCALE);
    regs_->risk_aversion = static_cast<uint64_t>(risk_aversion * SCALE);
    regs_->max_pos_limit = static_cast<uint64_t>(max_pos);
    
    // In a real system, we might trigger an interrupt or write a 'Commit' bit here
    // std::cout << "[FPGA] Updated Params: Skew=" << skew << " Gamma=" << risk_aversion << std::endl;
}

void FPGADriver::send_order(const strategy::StrategyOrder& order) {
    // Write Order to FPGA Registers (MMIO)
    regs_->order_inject_px = order.price;
    regs_->order_inject_qty = order.quantity;
    regs_->order_inject_side = (order.side == Side::BUY ? 1 : 0);
    
    // Trigger Execution (Write 1 to trigger register)
    // The FPGA logic would see this bit, read the params, and dispatch OUCH packet immediately.
    regs_->order_inject_trigger = 1; 
    
    // Simulation: Increment execution count
    regs_->execution_count++;
    
    // std::cout << "[FPGA] EXECUTION DISPATCHED: " << (order.side == Side::BUY ? "BUY" : "SELL") 
    //           << " " << order.quantity << "@" << order.price << std::endl;
}

int64_t FPGADriver::get_fpga_inventory() const {
    // In simulation, we just read what we wrote or a simulated value
    // Real hardware would write to this address via DMA/MMIO
    return regs_->fpga_inventory;
}

uint64_t FPGADriver::get_execution_count() const {
    return regs_->execution_count;
}

void FPGADriver::write_reg(size_t offset, uint64_t value) {
    if (offset + 8 > simulated_memory_.size()) return;
    *reinterpret_cast<uint64_t*>(&simulated_memory_[offset]) = value;
}

uint64_t FPGADriver::read_reg(size_t offset) const {
    if (offset + 8 > simulated_memory_.size()) return 0;
    return *reinterpret_cast<const uint64_t*>(&simulated_memory_[offset]);
}

} // namespace ultra::fpga
