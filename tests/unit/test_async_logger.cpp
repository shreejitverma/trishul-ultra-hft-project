#include "ultra/core/async_logger.hpp"
#include <iostream>
#include <fstream>
#include <chrono>

using namespace ultra;

int main() {
    std::cout << "[Test] Starting AsyncLogger Test...\n";

    std::string test_log = "test_run.log";
    AsyncLogger::instance().start(test_log);

    // Log some messages
    ULTRA_LOG("Hello HFT World! %d", 1);
    ULTRA_LOG("Processing Order %lu", 123456789UL);
    ULTRA_LOG("Risk Check Passed: %s", "TRUE");

    // Give it time to flush
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    AsyncLogger::instance().stop();

    // Verify file content
    std::ifstream in(test_log);
    std::string line;
    int line_count = 0;
    while (std::getline(in, line)) {
        line_count++;
        std::cout << "  Log: " << line << "\n";
    }

    if (line_count < 3) {
        std::cerr << "[FAIL] Expected 3 lines in log, got " << line_count << "\n";
        return 1;
    }

    std::cout << "[Test] AsyncLogger Test Passed.\n";
    return 0;
}
