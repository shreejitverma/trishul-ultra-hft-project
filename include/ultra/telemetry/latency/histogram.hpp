#pragma once
#include <atomic>
#include <array>
#include <cstdint>

namespace ultra::telemetry {

class LatencyHistogram {
public:
    static constexpr size_t NUM_BUCKETS = 1000; // 100ns * 1000 = 100us max
    
         /**
          * @brief Auto-generated description for record.
          * @param latency_ns Parameter description.
          */
    void record(uint64_t latency_ns);
         /**
          * @brief Auto-generated description for print_stats.
          */
    void print_stats() const;

private:
    std::array<std::atomic<uint64_t>, NUM_BUCKETS> buckets_{};
    std::atomic<uint64_t> count_{0}; ///< int variable representing count_.
    std::atomic<uint64_t> sum_ns_{0}; ///< int variable representing sum_ns_.
};

} // namespace ultra::telemetry
