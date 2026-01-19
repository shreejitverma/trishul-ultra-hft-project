#include "ultra/telemetry/latency/histogram.hpp"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace ultra::telemetry {

void LatencyHistogram::record(uint64_t latency_ns) {
    // 1. Determine Bucket
    // Simple linear bucketing for prototype: 0-100ns, 100-200ns...
    // Or log-scale. Let's use 100ns buckets up to 10us.
    
    size_t bucket_idx = latency_ns / 100;
    if (bucket_idx >= NUM_BUCKETS) bucket_idx = NUM_BUCKETS - 1;
    
    // 2. Increment (Relaxed atomics for speed)
    buckets_[bucket_idx].fetch_add(1, std::memory_order_relaxed);
    
    // 3. Update Min/Max
    // (Requires atomic CAS loop for perfect accuracy, but usually we just want approx stats)
    // We'll skip atomic min/max for this lock-free stub to keep it fast.
    count_.fetch_add(1, std::memory_order_relaxed);
    sum_ns_.fetch_add(latency_ns, std::memory_order_relaxed);
}

void LatencyHistogram::print_stats() const {
    uint64_t total = count_.load();
    if (total == 0) return;
    
    std::cout << "--- Latency Statistics ---" << std::endl;
    std::cout << "Total Samples: " << total << std::endl;
    std::cout << "Avg Latency: " << (sum_ns_.load() / total) << " ns" << std::endl;
    
    // Calculate P99 (Approx)
    uint64_t p99_threshold = static_cast<uint64_t>(total * 0.99);
    uint64_t accum = 0;
    for (size_t i = 0; i < NUM_BUCKETS; ++i) {
        accum += buckets_[i].load();
        if (accum >= p99_threshold) {
            std::cout << "P99 Latency: ~" << (i * 100) << " ns" << std::endl;
            break;
        }
    }
}

} // namespace ultra::telemetry
