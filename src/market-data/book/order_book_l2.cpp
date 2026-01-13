#include "ultra/market-data/book/order_book_l2.hpp"
#include <iostream>

namespace ultra::md {

OrderBookL2::OrderBookL2(SymbolId symbol_id) : symbol_id_(symbol_id) {
    // Initialize levels
    for (auto& level : bids_) { level.price = 0; level.quantity = 0; }
    for (auto& level : asks_) { level.price = INVALID_PRICE; level.quantity = 0; }
}

ULTRA_HOT void OrderBookL2::update(const itch::ITCHDecoder::DecodedMessage& msg) noexcept {
    if (ULTRA_UNLIKELY(!msg.valid)) return;
    if (msg.symbol_id != INVALID_SYMBOL && msg.symbol_id != symbol_id_) return;

    switch(msg.event_type) {
        case MDEventType::ADD_ORDER:
            add_order(msg.order_id, msg.side, msg.price, msg.quantity);
            break;
        case MDEventType::DELETE_ORDER:
            delete_order(msg.order_id);
            break;
        case MDEventType::MODIFY_ORDER:
            // This is complex for a map because we need to know the original side
            // For now, we ignore replaces or handle them simply as Delete + Add if we had the info
            // Since we don't have the original side in the message (ITCH Replace doesn't have it),
            // we'd need to look it up.
            {
                auto it = order_map_.find(msg.order_id);
                if (it != order_map_.end()) {
                    // Copy details before delete
                    Side old_side = it->second.side;
                    delete_order(msg.order_id);
                    // Add new
                    add_order(msg.new_order_id, old_side, msg.price, msg.quantity);
                }
            }
            break;
        default:
            break;
    }
    update_l2_view();
}

void OrderBookL2::add_order(OrderId id, Side side, Price price, Quantity qty) noexcept {
    // 1. Store L3
    order_map_[id] = {price, qty, side};

    // 2. Update Levels Map
    if (side == Side::BUY) {
        bid_levels_map_[price] += qty;
    } else {
        ask_levels_map_[price] += qty;
    }
}

void OrderBookL2::delete_order(OrderId id) noexcept {
    auto it = order_map_.find(id);
    if (it == order_map_.end()) return;

    const auto& order = it->second;
    if (order.side == Side::BUY) {
        auto level_it = bid_levels_map_.find(order.price);
        if (level_it != bid_levels_map_.end()) {
            level_it->second -= order.quantity;
            if (level_it->second <= 0) {
                bid_levels_map_.erase(level_it);
            }
        }
    } else {
        auto level_it = ask_levels_map_.find(order.price);
        if (level_it != ask_levels_map_.end()) {
            level_it->second -= order.quantity;
            if (level_it->second <= 0) {
                ask_levels_map_.erase(level_it);
            }
        }
    }
    order_map_.erase(it);
}

void OrderBookL2::modify_order(OrderId id, Quantity new_qty, Price new_price) noexcept {
    // Placeholder
    (void)id; (void)new_qty; (void)new_price;
}

void OrderBookL2::update_l2_view() noexcept {
    // Copy top levels from maps to arrays
    size_t i = 0;
    for (const auto& [price, qty] : bid_levels_map_) {
        if (i >= MAX_LEVELS) break;
        bids_[i].price = price;
        bids_[i].quantity = qty;
        i++;
    }
    // Fill remaining with 0
    for (; i < MAX_LEVELS; ++i) { bids_[i].price = 0; bids_[i].quantity = 0; }

    i = 0;
    for (const auto& [price, qty] : ask_levels_map_) {
        if (i >= MAX_LEVELS) break;
        asks_[i].price = price;
        asks_[i].quantity = qty;
        i++;
    }
    // Fill remaining with INVALID_PRICE
    for (; i < MAX_LEVELS; ++i) { asks_[i].price = INVALID_PRICE; asks_[i].quantity = 0; }
}

} // namespace ultra::md