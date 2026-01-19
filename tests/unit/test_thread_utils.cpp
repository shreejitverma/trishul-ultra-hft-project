#include "ultra/core/thread_utils.hpp"
#include <iostream>

using namespace ultra;

int main() {
    std::cout << "[Test] Starting ThreadUtils Test...\n";

    // Test Pinning (Should be safe to call even if it fails)
    std::cout << "  -> Attempting to pin to core 0...\n";
    ThreadUtils::pin_thread(0);

    // Test Priority (Should be safe to call)
    std::cout << "  -> Attempting to set RT priority...\n";
    ThreadUtils::set_realtime_priority(50);

    // Test Isolation
    std::cout << "  -> Attempting isolation on core 0...\n";
    ThreadUtils::isolate_thread(0, 50);

    std::cout << "[Test] ThreadUtils Test Passed (APIs callable).\n";
    return 0;
}
