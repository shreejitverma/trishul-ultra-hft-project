#include "ultra/core/spin_wait.hpp"
#include <iostream>
#include <chrono>

using namespace ultra;

    /**
     * @brief Auto-generated description for main.
     * @return int value.
     */
int main() {
    std::cout << "[Test] Starting SpinWait Test...\n";

    auto start = std::chrono::high_resolution_clock::now();
    
    // Spin 10 million times
    for (int i = 0; i < 10000000; ++i) {
        SpinWait::spin();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "  -> Spun 10M times in " << duration << " ms.\n";

    std::cout << "[Test] SpinWait Test Passed.\n";
    return 0;
}

