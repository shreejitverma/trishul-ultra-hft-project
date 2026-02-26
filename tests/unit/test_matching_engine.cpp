#include "ultra/execution/matching_engine.hpp"
#include <iostream>
#include <cassert>

using namespace ultra;
using namespace ultra::exec;

    /**
     * @brief Auto-generated description for main.
     * @return int value.
     */
int main() {
    std::cout << "[Test] Starting MatchingEngine Test...\n";
    MatchingEngine engine;

    // 1. Add Resting Sell Order (Ask)
    // Sell 100 @ 101.00
    strategy::StrategyOrder ask1;
    ask1.side = Side::SELL;
    ask1.price = 10100;
    ask1.quantity = 100;
    ask1.order_id = 1;
    ask1.type = OrderType::LIMIT;
    
    engine.process_order(ask1);
    
    if (engine.has_fills()) {
        std::cerr << "[FAIL] Resting order should not generate fills yet.\n";
        return 1;
    }

    // 2. Aggressive Buy Order (Cross)
    // Buy 50 @ 102.00 (Should match @ 101.00 - Price Improvement)
    strategy::StrategyOrder buy1;
    buy1.side = Side::BUY;
    buy1.price = 10200;
    buy1.quantity = 50;
    buy1.order_id = 2;
    buy1.type = OrderType::LIMIT; // or MARKET

    engine.process_order(buy1);

    if (!engine.has_fills()) {
        std::cerr << "[FAIL] Aggressive order should generate fill.\n";
        return 1;
    }

    ExecutionReport fill;
    engine.pop_fill(fill);
    
    std::cout << "  -> Fill: Qty=" << fill.fill_quantity << " Price=" << fill.fill_price << "\n";

    if (fill.fill_quantity != 50) {
        std::cerr << "[FAIL] Incorrect Fill Qty.\n";
        return 1;
    }
    if (fill.fill_price != 10100) {
        std::cerr << "[FAIL] Incorrect Fill Price (Should be passive price).\n";
        return 1;
    }

    std::cout << "[Test] MatchingEngine Test Passed.\n";
    return 0;
}
