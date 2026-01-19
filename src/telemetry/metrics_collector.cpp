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

void MetricsCollector::update_cpu_latency(uint64_t ns) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    cpu_stats_.count++;
    cpu_stats_.sum += ns;
    if (ns < cpu_stats_.min) cpu_stats_.min = ns;
    if (ns > cpu_stats_.max) cpu_stats_.max = ns;
}

void MetricsCollector::update_fpga_latency(uint64_t ns) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    fpga_stats_.count++;
    fpga_stats_.sum += ns;
    if (ns < fpga_stats_.min) fpga_stats_.min = ns;
    if (ns > fpga_stats_.max) fpga_stats_.max = ns;
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
    LatencyStats cpu_copy;
    LatencyStats fpga_copy;
    double cpu_load;
    double pnl_copy;

    {
        std::lock_guard<std::mutex> lock(data_mutex_);
        stats_copy = t2t_stats_;
        cpu_copy = cpu_stats_;
        fpga_copy = fpga_stats_;
        cpu_load = last_cpu_;
        pnl_copy = current_pnl_;
        
        // Reset per-second stats
        t2t_stats_ = LatencyStats(); 
        cpu_stats_ = LatencyStats();
        fpga_stats_ = LatencyStats();
    }

    if (stats_copy.count == 0 && cpu_copy.count == 0 && fpga_copy.count == 0) return; // No activity

    double avg_latency = (stats_copy.count > 0) ? static_cast<double>(stats_copy.sum) / stats_copy.count : 0.0;
    double avg_cpu_lat = (cpu_copy.count > 0) ? static_cast<double>(cpu_copy.sum) / cpu_copy.count : 0.0;
    double avg_fpga_lat = (fpga_copy.count > 0) ? static_cast<double>(fpga_copy.sum) / fpga_copy.count : 0.0;

    // InfluxDB Line Protocol:
    // measurement,tag_set field_set timestamp
    // system_metrics,host=fpga_server t2t_avg=...,t2t_min=...,cpu=...,pnl=...
    
    // Using simple stdout for simulation
    std::cout << "[InfluxDB] system_metrics,host=ultra_fpga "
              << "t2t_avg=" << avg_latency << ","
              << "t2t_min=" << stats_copy.min << ","
              << "cpu_exec_avg=" << avg_cpu_lat << ","
              << "fpga_exec_avg=" << avg_fpga_lat << ","
              << "cpu_load=" << cpu_load << ","
              << "pnl=" << pnl_copy << " "
              << std::chrono::duration_cast<std::chrono::nanoseconds>(
                  std::chrono::system_clock::now().time_since_epoch()).count()
              << "\n";
}

} // namespace ultra::telemetry
