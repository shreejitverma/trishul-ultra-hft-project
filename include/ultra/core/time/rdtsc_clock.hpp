#pragma once
#include "../compiler.hpp"
#include "../types.hpp"
#include <chrono>
#include <atomic>

#if defined(__x86_64__) || defined(_M_X64)
    #include <x86intrin.h>
#endif

namespace ultra {

/**
 * Ultra-low latency clock using RDTSC instruction
 * Calibrated to nanoseconds, ~20ns overhead
 */
class RDTSCClock {
public:
    static void calibrate() noexcept;
    
    // Get current time in nanoseconds (fastest path)
    ULTRA_ALWAYS_INLINE static Timestamp now() noexcept {
        return rdtsc_to_ns(rdtsc());
    }
    
    // Get raw TSC value
    ULTRA_ALWAYS_INLINE static uint64_t rdtsc() noexcept {
#if defined(__aarch64__)
        uint64_t val;
        asm volatile("mrs %0, cntvct_el0" : "=r" (val));
        return val;
#else
        return __rdtsc();
#endif
    }
    
    // Serializing RDTSC (more expensive but precise)
    ULTRA_ALWAYS_INLINE static uint64_t rdtscp() noexcept {
#if defined(__aarch64__)
        uint64_t val;
        // ISB ensures previous instructions complete before reading the timer
        asm volatile("isb; mrs %0, cntvct_el0" : "=r" (val) :: "memory");
        return val;
#else
        uint32_t aux;
        return __rdtscp(&aux);
#endif
    }
    
    // Convert TSC to nanoseconds
    ULTRA_ALWAYS_INLINE static Timestamp rdtsc_to_ns(uint64_t tsc) noexcept {
        return static_cast<Timestamp>(tsc * tsc_to_ns_factor_.load(std::memory_order_relaxed));
    }
    
    // Get system time (slower, for logging)
    static Timestamp system_now() noexcept;
    
private:
    static std::atomic<double> tsc_to_ns_factor_;
};

} // namespace ultra
