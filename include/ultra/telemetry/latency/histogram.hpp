#pragma once
#include <atomic>
#include <array>
#include <cstdint>

namespace ultra::telemetry {

class LatencyHistogram {
public:
    static constexpr size_t NUM_BUCKETS = 1000; // 100ns * 1000 = 100us max
    
    void record(uint64_t latency_ns);
    void print_stats() const;

private:
    std::array<std::atomic<uint64_t>, NUM_BUCKETS> buckets_{};
    std::atomic<uint64_t> count_{0};
    std::atomic<uint64_t> sum_ns_{0};
};

} // namespace ultra::telemetry
