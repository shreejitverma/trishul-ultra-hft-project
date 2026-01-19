#include "ultra/execution/router/sor.hpp"
#include "ultra/core/symbol_universe.hpp"
#include "ultra/fpga/fpga_driver.hpp"
#include "ultra/execution/gateway_sim.hpp"
#include <iostream>
#include <cassert>

using namespace ultra;

int main() {
    std::cout << "[Test] Starting Hybrid Routing Test...\n";

    // 1. Setup Universe
    auto& universe = SymbolUniverse::instance();
    universe.add_symbol({1, "FAST", 100, 1, 0, 0, true});  // FPGA
    universe.add_symbol({2, "SLOW", 100, 1, 0, 0, false}); // CPU

    // 2. Setup Components
    fpga::FPGADriver fpga;
    fpga.init();
    
    exec::GatewaySim gateway;
    
    execution::SmartOrderRouter router(&fpga, &gateway);

    // 3. Test FPGA Routing
    strategy::StrategyOrder fast_order;
    fast_order.symbol_id = 1;
    fast_order.price = 100;
    fast_order.quantity = 10;
    fast_order.side = Side::BUY;
    
    uint64_t fpga_count_before = fpga.get_execution_count();
    router.route(fast_order);
    uint64_t fpga_count_after = fpga.get_execution_count();
    
    if (fpga_count_after != fpga_count_before + 1) {
        std::cerr << "[FAIL] FPGA routing failed. Count didn't increment.\n";
        return 1;
    }
    std::cout << "  -> FPGA Routing OK.\n";

    // 4. Test CPU Routing
    strategy::StrategyOrder slow_order;
    slow_order.symbol_id = 2;
    slow_order.price = 200;
    slow_order.quantity = 20;
    slow_order.side = Side::SELL;
    
    router.route(slow_order);
    
    // Check Gateway (it queues a report)
    exec::ExecutionReport report;
    if (!gateway.get_execution_report(report)) {
        // It might take a moment or be immediate. Sim is usually immediate.
        // Wait, GatewaySim::send_order calls try_match -> send_fill/accept -> queue push.
        // So it should be there.
        // However, GatewaySim in "try_match" might not fill immediately if no crossing order?
        // But "send_order" sends "Accepted" message first.
        std::cerr << "[FAIL] CPU routing failed. No report generated.\n";
        return 1;
    }
    std::cout << "  -> CPU Routing OK.\n";

    std::cout << "[Test] Hybrid Routing Test Passed.\n";
    return 0;
}
