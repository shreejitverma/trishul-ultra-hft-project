#pragma once
#include "../../core/symbol_universe.hpp"
#include "../../fpga/fpga_driver.hpp"
#include "../gateway_sim.hpp"
#include "../../telemetry/metrics_collector.hpp"
#include "../../core/time/rdtsc_clock.hpp"
#include "../../strategy/strategy.hpp"

namespace ultra::execution {

class SmartOrderRouter {
public:
    /**
     * @brief Auto-generated description for SmartOrderRouter.
     * @param fpga Parameter description.
     * @param gateway Parameter description.
     */
    SmartOrderRouter(fpga::FPGADriver* fpga, exec::GatewaySim* gateway)
        : fpga_(fpga), gateway_(gateway) {}

         /**
          * @brief Auto-generated description for route.
          * @param order Parameter description.
          */
    void route(const strategy::StrategyOrder& order) {
        auto& universe = SymbolUniverse::instance();
        const SymbolInfo* info = universe.get_symbol(order.symbol_id);
        
        bool use_fpga = false;
        if (info && info->use_fpga_execution) {
            use_fpga = true;
        }

        uint64_t start = RDTSCClock::rdtsc();

        if (use_fpga && fpga_) {
            // Path A: FPGA Hardware Offload
            fpga_->send_order(order);
            
            uint64_t end = RDTSCClock::rdtsc();
            uint64_t latency = RDTSCClock::rdtsc_to_ns(end - start);
            telemetry::MetricsCollector::instance().update_fpga_latency(latency);
        } else if (gateway_) {
            // Path B: CPU Software Gateway
            gateway_->send_order(order);
            
            uint64_t end = RDTSCClock::rdtsc();
            uint64_t latency = RDTSCClock::rdtsc_to_ns(end - start);
            telemetry::MetricsCollector::instance().update_cpu_latency(latency);
        }
    }

private:
    fpga::FPGADriver* fpga_; ///< fpga::FPGADriver * variable representing fpga_.
    exec::GatewaySim* gateway_; ///< exec::GatewaySim * variable representing gateway_.
};

} // namespace ultra::execution