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

         /**
          * @brief Auto-generated description for start.
          */
    void start();
         /**
          * @brief Auto-generated description for stop.
          */
    void stop();

    // High-frequency updates (lock-free or minimal lock preferred, using spinlock here for simplicity)
    void update_t2t_latency(uint64_t ns);
    void update_cpu_latency(uint64_t ns);
    void update_fpga_latency(uint64_t ns);
         /**
          * @brief Auto-generated description for update_cpu_usage.
          * @param percent Parameter description.
          */
    void update_cpu_usage(double percent);
         /**
          * @brief Auto-generated description for update_pnl.
          * @param pnl Parameter description.
          */
    void update_pnl(double pnl);

private:
    /**
     * @brief Auto-generated description for MetricsCollector.
     */
    MetricsCollector() = default;
    /**
     * @brief Auto-generated description for ~MetricsCollector.
     */
    ~MetricsCollector();

         /**
          * @brief Auto-generated description for run_loop.
          */
    void run_loop();
         /**
          * @brief Auto-generated description for flush_to_influx.
          */
    void flush_to_influx();

    std::atomic<bool> running_{false}; ///< int variable representing running_.
    std::thread flusher_thread_; ///< int variable representing flusher_thread_.
    
    // Aggregates
    struct LatencyStats {
        uint64_t count{0}; ///< int variable representing count.
        uint64_t sum{0}; ///< int variable representing sum.
        uint64_t min{999999999}; ///< int variable representing min.
        uint64_t max{0}; ///< int variable representing max.
    };
    
    std::mutex data_mutex_; ///< int variable representing data_mutex_.
    LatencyStats t2t_stats_; ///< LatencyStats variable representing t2t_stats_.
    LatencyStats cpu_stats_; ///< LatencyStats variable representing cpu_stats_.
    LatencyStats fpga_stats_; ///< LatencyStats variable representing fpga_stats_.
    double last_cpu_{0.0}; ///< double variable representing last_cpu_.
    double current_pnl_{0.0}; ///< double variable representing current_pnl_.
};

} // namespace ultra::telemetry
