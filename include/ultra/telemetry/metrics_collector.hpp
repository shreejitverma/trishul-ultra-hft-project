#pragma once
#include <atomic>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include "ultra/core/types.hpp"

namespace ultra::telemetry {

/**
 * Centralized Metrics Collector (Fig 7)
 * Aggregates FPGA T2T latencies and Software metrics (CPU, PnL).
 * Pushes to InfluxDB (simulated).
 */
class MetricsCollector {
public:
    static MetricsCollector& instance() {
        static MetricsCollector instance;
        return instance;
    }

    void start();
    void stop();

    // High-frequency updates (lock-free or minimal lock preferred, using spinlock here for simplicity)
    void update_t2t_latency(uint64_t ns);
    void update_cpu_usage(double percent);
    void update_pnl(double pnl);

private:
    MetricsCollector() = default;
    ~MetricsCollector();

    void run_loop();
    void flush_to_influx();

    std::atomic<bool> running_{false};
    std::thread flusher_thread_;
    
    // Aggregates
    struct LatencyStats {
        uint64_t count{0};
        uint64_t sum{0};
        uint64_t min{999999999};
        uint64_t max{0};
    };
    
    std::mutex data_mutex_;
    LatencyStats t2t_stats_;
    double last_cpu_{0.0};
    double current_pnl_{0.0};
};

} // namespace ultra::telemetry
