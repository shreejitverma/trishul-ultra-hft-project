#include "ultra/core/async_logger.hpp"
#include <gtest/gtest.h>
#include <fstream>
#include <chrono>
#include <thread>
// #include <json/json.h> // Removed as it's not available

using namespace ultra;

class AsyncLoggerTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Ensure clean state by deleting potential leftover log files
        std::remove("/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/test_standard.log");
        std::remove("/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/test_json.log");
        std::remove("/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/test_stress.log");
    }
    void TearDown() override {
        AsyncLogger::instance().stop();
    }
};

TEST_F(AsyncLoggerTest, StandardLogging) {
    std::string test_log = "/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/test_standard.log";
    AsyncLogger::instance().start(test_log, false);

    ULTRA_LOG(INFO, "Info message %d", 1);
    ULTRA_LOG(WARN, "Warn message %s", "test");
    ULTRA_LOG(ERROR, "Error message");
    ULTRA_LOG(DEBUG, "Debug message");

    // Test Flow and Return
    {
        ULTRA_TRACE("TestFunc", "Testing Return", "val=42");
        ULTRA_TRACE_RET(42);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    AsyncLogger::instance().stop();

    std::ifstream in(test_log);
    std::string line;
    bool found_return = false;
    while (std::getline(in, line)) {
        if (line.find("RETURN -> 42") != std::string::npos) found_return = true;
    }
    EXPECT_TRUE(found_return);
}

TEST_F(AsyncLoggerTest, JsonLogging) {
    std::string test_log = "/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/test_json.log";
    AsyncLogger::instance().start(test_log, true);

    ULTRA_LOG(INFO, "Json message");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    AsyncLogger::instance().stop();

    std::ifstream in(test_log);
    std::string line;
    std::getline(in, line);
    
    EXPECT_FALSE(line.empty());
    EXPECT_EQ(line[0], '{');
    EXPECT_NE(line.find("\"lvl\":\"INFO\""), std::string::npos);
    EXPECT_NE(line.find("\"msg\":\"Json message\""), std::string::npos);
}

TEST_F(AsyncLoggerTest, StressTest) {
    std::string test_log = "/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/test_stress.log";
    AsyncLogger::instance().start(test_log, false);

    const int num_msgs = 10000;
    for(int i = 0; i < num_msgs; ++i) {
        ULTRA_LOG(INFO, "Stress test message %d", i);
    }

    AsyncLogger::instance().stop(); // Stop will flush remaining
    
    std::ifstream in(test_log);
    int count = 0;
    std::string line;
    while (std::getline(in, line)) count++;
    
    EXPECT_EQ(count, num_msgs);
}
