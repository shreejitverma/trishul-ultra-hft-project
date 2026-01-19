#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <cstring>
#include <iostream>
#include "../compiler.hpp"

#if defined(__linux__)
#include <sys/mman.h>
#endif

namespace ultra {

class MemoryUtils {
public:
    /**
     * Locks all current and future memory to RAM to prevent swapping.
     * Requires CAP_IPC_LOCK or root.
     */
    static void lock_memory() {
#if defined(__linux__)
        if (mlockall(MCL_CURRENT | MCL_FUTURE) != 0) {
             std::cerr << "Failed to lock memory (mlockall). Need root/CAP_IPC_LOCK?\n";
        }
#else
        // macOS: mlockall is not standard or behaves differently
#endif
    }

    /**
     * Writes to every 4KB page in the buffer to force physical allocation (page fault) now.
     */
    static void prefault_buffer(void* ptr, size_t size) {
        volatile uint8_t* p = static_cast<uint8_t*>(ptr);
        const size_t page_size = 4096;
        for (size_t i = 0; i < size; i += page_size) {
            p[i] = 0;
        }
        // Touch last byte
        if (size > 0) {
            p[size - 1] = 0;
        }
    }

    /**
     * Expands the stack by 256KB to ensure pages are mapped.
     */
    static void warmup_stack() {
        const size_t STACK_SIZE = 256 * 1024; // 256KB
        volatile uint8_t buffer[STACK_SIZE];
        prefault_buffer((void*)buffer, STACK_SIZE);
    }
};

} // namespace ultra

