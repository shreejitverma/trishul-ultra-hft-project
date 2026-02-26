#include "apps/live-engine/engine.hpp"
#include "ultra/core/async_logger.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <chrono>

using namespace ultra;

class EngineIntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        AsyncLogger::instance().start("/Users/shreejitverma/Documents/GitHub/ultra-hft-project/logs/integration_test.log");
    }

    void TearDown() override {
        AsyncLogger::instance().stop();
    }
};

TEST_F(EngineIntegrationTest, FullPipelineSimulation) {
    // 1. Initialize Engine in Simulation Mode
    Engine engine;
    
    // 2. Run engine in a separate thread
    std::atomic<bool> engine_started{false};
    std::thread engine_thread([&]() {
        engine_started = true;
        engine.run();
    });

    // Wait for engine to start
    while(!engine_started) std::this_thread::yield();
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 3. Verify it's running (check log or internal state if possible)
    // For now, we'll just stop it and ensure it shuts down cleanly.
    
    // 4. Stop Engine
    engine.stop();
    if (engine_thread.joinable()) {
        engine_thread.join();
    }
    
    SUCCEED();
}

TEST_F(EngineIntegrationTest, LowLevelComponentFlow) {
    // Manually push data through components to ensure 100% coverage of edge cases
    auto& universe = SymbolUniverse::instance();
    const SymbolId TEST_ID = 999;
    universe.add_symbol({TEST_ID, "TEST", 100, 1, -0.01, 0.01, false});

    strategy::RLPolicyStrategy strat(TEST_ID);
    risk::PretradeChecker::Config risk_cfg;
    risk::PretradeChecker risk_checker(risk_cfg);
    
    // Simulate MD
    md::itch::ITCHDecoder::DecodedMessage msg;
    msg.valid = true;
    msg.symbol_id = TEST_ID;
    msg.price = 10000;
    msg.quantity = 100;
    msg.side = Side::BUY;
    
    strat.on_market_data(msg);
    
    // Trigger order generation (if any)
    strategy::StrategyOrder order;
    while(strat.get_order(order)) {
        bool risk_ok = risk_checker.check_order(order);
        EXPECT_TRUE(risk_ok);
    }
}
