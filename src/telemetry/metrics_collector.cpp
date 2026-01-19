#include "ultra/telemetry/metrics_collector.hpp"
#include <iostream>
#include <chrono>

namespace ultra::telemetry {

MetricsCollector::~MetricsCollector() {
    stop();
}

void MetricsCollector::start() {
    if (running_) return;
    running_ = true;
    flusher_thread_ = std::thread(&MetricsCollector::run_loop, this);
}

void MetricsCollector::stop() {
    if (!running_) return;
    running_ = false;
    if (flusher_thread_.joinable()) {
        flusher_thread_.join();
    }
}

void MetricsCollector::update_t2t_latency(uint64_t ns) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    t2t_stats_.count++;
    t2t_stats_.sum += ns;
    if (ns < t2t_stats_.min) t2t_stats_.min = ns;
    if (ns > t2t_stats_.max) t2t_stats_.max = ns;
}

void MetricsCollector::update_cpu_usage(double percent) {
    // CPU usage is usually sampled, just store last
    std::lock_guard<std::mutex> lock(data_mutex_);
    last_cpu_ = percent;
}

void MetricsCollector::update_pnl(double pnl) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    current_pnl_ = pnl;
}

void MetricsCollector::run_loop() {
    while (running_) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        flush_to_influx();
    }
}

void MetricsCollector::flush_to_influx() {
    LatencyStats stats_copy;
    double cpu_copy;
    double pnl_copy;

    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        stats_copy = t2t_stats_;
        cpu_copy = last_cpu_;
        pnl_copy = current_pnl_;
        
        // Reset per-second stats
        t2t_stats_ = LatencyStats(); 
    }

    if (stats_copy.count == 0) return; // No activity

    double avg_latency = static_cast<double>(stats_copy.sum) / stats_copy.count;

    // InfluxDB Line Protocol:
    // measurement,tag_set field_set timestamp
    // system_metrics,host=fpga_server t2t_avg=...,t2t_min=...,cpu=...,pnl=...
    
    // Using simple stdout for simulation
    std::cout << "[InfluxDB] system_metrics,host=ultra_fpga "
              << "t2t_avg=" << avg_latency << ","
              << "t2t_min=" << stats_copy.min << ","
              << "t2t_max=" << stats_copy.max << ","
              << "cpu_load=" << cpu_copy << ","
              << "pnl=" << pnl_copy << " "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count()
              << "\n";
}

} // namespace ultra::telemetry
