#include "ultra/core/memory/memory_utils.hpp"
#include <iostream>
#include <vector>

using namespace ultra;

int main() {
    std::cout << "[Test] Starting MemoryUtils Test...\n";

    // 1. Test Lock Memory
    std::cout << "  -> Attempting mlockall...\n";
    MemoryUtils::lock_memory();

    // 2. Test Prefault Buffer
    std::cout << "  -> Prefaulting 1MB buffer...\n";
    std::vector<uint8_t> buffer(1024 * 1024);
    MemoryUtils::prefault_buffer(buffer.data(), buffer.size());

    // 3. Test Stack Warmup
    std::cout << "  -> Warming up stack...\n";
    MemoryUtils::warmup_stack();

    std::cout << "[Test] MemoryUtils Test Passed.\n";
    return 0;
}
