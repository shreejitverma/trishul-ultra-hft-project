#include "ultra/risk/pretrade_checker.hpp"
#include "ultra/execution/gateway_sim.hpp"
#include "ultra/core/async_logger.hpp"
#include <iostream>

namespace ultra::risk {

                 /**
                  * @brief Auto-generated description for PretradeChecker.
                  * @param config Parameter description.
                  */
PretradeChecker::PretradeChecker(const Config& config) : config_(config) {
    ULTRA_TRACE_SIMPLE("PretradeChecker::PretradeChecker");
}

                                /**
                                 * @brief Auto-generated description for check_order.
                                 * @param order Parameter description.
                                 * @return bool value.
                                 */
ULTRA_HOT bool PretradeChecker::check_order(const strategy::StrategyOrder& order) noexcept {
    ULTRA_TRACE("PretradeChecker::check_order", "Safety validation before routing", 
                "oid=" + std::to_string(order.order_id) + " qty=" + std::to_string(order.quantity));
    
    uint64_t now_ns = RDTSCClock::now();

    // Check 1: Fat-Finger (Size and Price)
    if (ULTRA_UNLIKELY(order.quantity > config_.max_order_size)) {
        ULTRA_LOG(WARN, "RISK REJECT: Max order size violation.");
        return ULTRA_TRACE_RET(false);
    }
    if (ULTRA_UNLIKELY(order.price > config_.max_price || order.price < config_.min_price)) {
        ULTRA_LOG(WARN, "RISK REJECT: Fat-finger price violation.");
        return ULTRA_TRACE_RET(false);
    }

    // Check 2: Credit Limit (Position and Notional)
    Quantity new_position = current_position_;
    if (order.side == Side::BUY) {
        new_position += order.quantity;
    } else {
        new_position -= order.quantity;
    }

    if (ULTRA_UNLIKELY(std::abs(new_position) > config_.max_position_shares)) {
         ULTRA_LOG(WARN, "RISK REJECT: Max position shares violation.");
        return ULTRA_TRACE_RET(false);
    }

    Price order_notional = order.price * order.quantity;
    if (ULTRA_UNLIKELY(total_notional_exposure_ + order_notional > config_.max_notional_usd * PRICE_SCALE)) {
        ULTRA_LOG(WARN, "RISK REJECT: Gross notional exposure limit reached.");
        return ULTRA_TRACE_RET(false);
    }

    // Check 3: Duplicate Order Detection (sub-microsecond window)
    // Simple hash: side | price | qty (mock)
    uint64_t current_hash = (static_cast<uint64_t>(order.side) << 48) ^ (order.price << 16) ^ order.quantity;
    if (ULTRA_UNLIKELY(current_hash == last_order_hash_ && (now_ns - last_order_time_ns_ < config_.duplicate_window_ns))) {
        ULTRA_LOG(WARN, "RISK REJECT: Duplicate order detected in sub-us window.");
        return ULTRA_TRACE_RET(false);
    }
    
    // All checks passed - Update internal state
    total_notional_exposure_ += order_notional;
    last_order_hash_ = current_hash;
    last_order_time_ns_ = now_ns;

    return ULTRA_TRACE_RET(true);
}

                      /**
                       * @brief Auto-generated description for on_execution.
                       * @param report Parameter description.
                       */
void PretradeChecker::on_execution(const exec::ExecutionReport& report) noexcept {
    ULTRA_TRACE("PretradeChecker::on_execution", "Position update from fill", "oid=" + std::to_string(report.order_id));
    // Update position on fills
    if (report.status == OrderStatus::FILLED || report.status == OrderStatus::PARTIAL) {
        // Mock logic
    }
}

} // namespace ultra::risk