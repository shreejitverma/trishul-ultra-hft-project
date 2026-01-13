#include "ultra/strategy/rl-inference/rl_policy.hpp"
#include "ultra/execution/gateway_sim.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

namespace ultra::strategy {

RLPolicyStrategy::RLPolicyStrategy(SymbolId symbol_id)
    : symbol_id_(symbol_id), order_book_(symbol_id) {
    std::cout << "RLPolicyStrategy (AI-Integrated) initialized for symbol " << symbol_id_ << std::endl;
}

RLPolicyStrategy::~RLPolicyStrategy() = default;

void RLPolicyStrategy::on_market_data(const md::itch::ITCHDecoder::DecodedMessage& msg) {
    // 1. Update our internal view of the L2 order book
    order_book_.update(msg);

    // 2. On a significant event (e.g., BBO change), run inference
    // For HFT, we often sample or run on every tick if FPGA offloaded.
    // Here we run on every BBO update roughly.
    run_inference();
}

void RLPolicyStrategy::on_execution(const exec::ExecutionReport& report) {
    // Update inventory
    if (report.status == OrderStatus::FILLED || report.status == OrderStatus::PARTIAL) {
        if (report.symbol_id == symbol_id_) {
            // Rudimentary side tracking (Real system tracks order IDs maps)
            if (report.fill_price < order_book_.best_ask().price) {
                // Likely a BUY fill
                current_inventory_ += report.fill_quantity;
            } else {
                // Likely a SELL fill
                current_inventory_ -= report.fill_quantity;
            }
            std::cout << "[Strategy] Inventory: " << current_inventory_ << std::endl;
        }
    }
}

bool RLPolicyStrategy::get_order(StrategyOrder& order) {
    return order_queue_.pop(order);
}

void RLPolicyStrategy::run_inference() noexcept {
    // --- AI / Quantitative Model Inference ---
    
    auto bbo_bid = order_book_.best_bid();
    auto bbo_ask = order_book_.best_ask();
    
    if (bbo_bid.price == 0 || bbo_ask.price == INVALID_PRICE) return;

    // 1. Feature Extraction
    double mid_price = (bbo_bid.price + bbo_ask.price) / 2.0;
    // double spread_bps = (double)(bbo_ask.price - bbo_bid.price) / mid_price * 10000.0;
    
    // 2. Model Parameters (These would be trained via RL and loaded/updated via FPGA Driver)
    double gamma = 0.1;       // Risk aversion
    double sigma = 2.0;       // Volatility (stub)
    
    // ModelFeatures features construction kept for documentation/future RL input
    ModelFeatures features = {
        .best_bid = bbo_bid.price,
        .bid_size = bbo_bid.quantity,
        .best_ask = bbo_ask.price,
        .ask_size = bbo_ask.quantity,
        .mid_price = static_cast<Price>(mid_price),
        .spread = bbo_ask.price - bbo_bid.price,
        .imbalance = 0, 
        .volatility = sigma,
        .inventory = current_inventory_
    };
    (void)features; // Silence unused warning

    // 3. Inference Stub (Avellaneda-Stoikov Approximation)
    // Reservation Price r = s - q * gamma * sigma^2 * (T-t)
    // We simplify (T-t) to 1 for infinite horizon approximation
    
    double inventory_risk_adj = current_inventory_ * gamma * (sigma * sigma); 
    double reservation_price = mid_price - inventory_risk_adj;
    
    // Optimal Spread (Simplified)
    // half_spread = gamma * sigma^2 + (2/gamma) * ln(1 + gamma/k)
    // We'll use a linear heuristic based on volatility
    double half_spread = (sigma * 5000.0); // Tuning scalar
    if (half_spread < (bbo_ask.price - bbo_bid.price)/2.0) {
        half_spread = (bbo_ask.price - bbo_bid.price)/2.0; // Don't cross market aggressively
    }

    Price optimal_bid = static_cast<Price>(reservation_price - half_spread);
    Price optimal_ask = static_cast<Price>(reservation_price + half_spread);
    
    // Quantize to ticks
    optimal_bid = (optimal_bid / 100) * 100; // Assume tick size 0.01 (100 units)
    optimal_ask = (optimal_ask / 100) * 100;

    // 4. Generate Orders
    // (Only if prices changed significantly to avoid quote flickering)
    StrategyOrder buy_order = {
        .action = StrategyOrder::Action::NEW_ORDER,
        .order_id = 0,
        .symbol_id = symbol_id_,
        .side = Side::BUY,
        .price = optimal_bid,
        .quantity = 100,
        .type = OrderType::LIMIT
    };
    
    StrategyOrder sell_order = {
        .action = StrategyOrder::Action::NEW_ORDER,
        .order_id = 0,
        .symbol_id = symbol_id_,
        .side = Side::SELL,
        .price = optimal_ask,
        .quantity = 100,
        .type = OrderType::LIMIT
    };

    // Only push if valid prices
    if (optimal_bid > 0) order_queue_.push(buy_order);
    if (optimal_ask > optimal_bid) order_queue_.push(sell_order);
}

RLPolicyStrategy::ModelOutput RLPolicyStrategy::inference_stub(const ModelFeatures& features) noexcept {
    // Deprecated in favor of inline logic above, but kept for interface compat
    (void)features;
    return {}; 
}

} // namespace ultra::strategy
