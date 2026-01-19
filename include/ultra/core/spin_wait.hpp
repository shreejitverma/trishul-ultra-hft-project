#pragma once
#include "compiler.hpp"
#include <thread>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#endif

namespace ultra {

class SpinWait {
public:
    /**
     * Executes a CPU-relax instruction (pause/yield).
     * Prevents the CPU from speculating past this point and saves power
     * without yielding the OS timeslice (latency killer).
     */
    ULTRA_ALWAYS_INLINE static void spin() {
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
        _mm_pause();
#elif defined(__aarch64__) || defined(_M_ARM64)
        // ARM64 yield (often treated as a hint)
        asm volatile("yield");
#else
        // Fallback for unknown arch (soft yield)
        std::this_thread::yield();
#endif
    }
};

} // namespace ultra
