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
    
    // Check 1: Max Order Size
    if (ULTRA_UNLIKELY(order.quantity > config_.max_order_size)) {
        ULTRA_LOG(WARN, "RISK REJECT: Max order size. %ld > %ld", order.quantity, config_.max_order_size);
        return ULTRA_TRACE_RET(false);
    }

    // Check 2: Max Position
    Quantity new_position = current_position_;
    if (order.side == Side::BUY) {
        new_position += order.quantity;
    } else {
        new_position -= order.quantity;
    }

    if (ULTRA_UNLIKELY(std::abs(new_position) > config_.max_position_shares)) {
         ULTRA_LOG(WARN, "RISK REJECT: Max position. %ld > %ld", new_position, config_.max_position_shares);
        return ULTRA_TRACE_RET(false);
    }

    // Check 3: Max Notional
    Price notional = order.price * order.quantity;
    if (ULTRA_UNLIKELY(notional > config_.max_notional_usd * PRICE_SCALE)) {
        ULTRA_LOG(WARN, "RISK REJECT: Max notional.");
        return ULTRA_TRACE_RET(false);
    }
    
    // All checks passed
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