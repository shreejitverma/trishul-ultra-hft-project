#pragma once
#include "../core/types.hpp"
#include "../strategy/strategy.hpp"

namespace ultra::risk {

/**
 * This is the hardware "Risk Engines" from your thesis, Fig 3 [cite: 101]
 * Implemented in C++ for the software path.
 */
class PretradeChecker {
public:
    struct Config {
        Quantity max_position_shares = 10000;
        Price max_notional_usd = 10000000;
        Quantity max_order_size = 1000;
        uint32_t max_orders_per_second = 10000;
        Price max_price = 5000 * PRICE_SCALE; // $5000
        Price min_price = 1 * PRICE_SCALE;    // $1
        uint64_t duplicate_window_ns = 1000;   // 1us
    };

    explicit PretradeChecker(const Config& config);

    ULTRA_HOT bool check_order(const strategy::StrategyOrder& order) noexcept;

    void on_execution(const exec::ExecutionReport& report) noexcept;

private:
    Config config_;
    
    Quantity current_position_{0};
    Price total_notional_exposure_{0};
    
    // Duplicate Detection State
    uint64_t last_order_hash_{0};
    uint64_t last_order_time_ns_{0};
};

} // namespace ultra::risk
