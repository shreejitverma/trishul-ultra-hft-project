#include "ultra/market-data/book/order_book_l2.hpp"
#include <iostream>
#include <cstring> // for memmove
#include <chrono>

namespace ultra::md {

OrderBookL2::OrderBookL2(SymbolId symbol_id) : symbol_id_(symbol_id) {
    // Clear hash map
    std::fill(order_map_.begin(), order_map_.end(), nullptr);
    
    // Clear levels
    for (auto& level : bids_) { level.price = 0; level.quantity = 0; level.order_count = 0; }
    for (auto& level : asks_) { level.price = INVALID_PRICE; level.quantity = 0; level.order_count = 0; }
}

ULTRA_HOT void OrderBookL2::update(const itch::ITCHDecoder::DecodedMessage& msg) noexcept {
    if (ULTRA_UNLIKELY(!msg.valid)) return;
    if (msg.symbol_id != INVALID_SYMBOL && msg.symbol_id != symbol_id_) return;

    // Capture BBO state before update
    // We only care about the top level [0]
    const auto prev_bid_price = bids_[0].price;
    const auto prev_bid_qty = bids_[0].quantity;
    const auto prev_ask_price = asks_[0].price;
    const auto prev_ask_qty = asks_[0].quantity;

    switch(msg.event_type) {
        case MDEventType::ADD_ORDER:
            add_order(msg.order_id, msg.side, msg.price, msg.quantity);
            break;
        case MDEventType::DELETE_ORDER:
            delete_order(msg.order_id);
            break;
        case MDEventType::MODIFY_ORDER:
            // Partial implementation for replace (delete + add new)
            // Real ITCH "Order Replace" replaces the order *in place* if size decreases,
            // or loses priority if size increases. 
            // We need to look up the old order to know its side/price.
            {
                uint32_t h = hash(msg.order_id);
                OrderEntry* curr = order_map_[h];
                while (curr) {
                    if (curr->id == msg.order_id) {
                        // Found old order
                        Side side = curr->side;
                        // Price price = curr->price; // Unused
                        // Delete old
                        delete_order(msg.order_id);
                        // Add new
                        add_order(msg.new_order_id, side, msg.price, msg.quantity); // Note: msg.price is new price
                        goto bbo_check; // Break out of loop and switch
                    }
                    curr = curr->next;
                }
            }
            break;
        default:
            break;
    }

bbo_check:
    // Check for BBO changes
    if (ULTRA_LIKELY(listener_)) {
        if (bids_[0].price != prev_bid_price || bids_[0].quantity != prev_bid_qty ||
            asks_[0].price != prev_ask_price || asks_[0].quantity != prev_ask_qty) {
            
            // Get current timestamp
            auto now = std::chrono::steady_clock::now().time_since_epoch();
            uint64_t ts = std::chrono::duration_cast<std::chrono::nanoseconds>(now).count();

            listener_({
                symbol_id_,
                bids_[0].price,
                bids_[0].quantity,
                asks_[0].price,
                asks_[0].quantity,
                ts
            });
        }
    }
}

void OrderBookL2::add_order(OrderId id, Side side, Price price, Quantity qty) noexcept {
    // 1. Allocate from pool
    OrderEntry* new_order = order_pool_.allocate();
    if (ULTRA_UNLIKELY(!new_order)) {
        // Pool full - in prod we might log or crash. 
        // For now, silently drop to avoid segfault.
        return;
    }
    new_order->id = id;
    new_order->side = side;
    new_order->price = price;
    new_order->quantity = qty;
    new_order->next = nullptr;

    // 2. Insert into Hash Map (Chaining)
    uint32_t h = hash(id);
    new_order->next = order_map_[h];
    order_map_[h] = new_order;

    // 3. Update Levels
    update_level(side, price, static_cast<int32_t>(qty));
}

void OrderBookL2::delete_order(OrderId id) noexcept {
    uint32_t h = hash(id);
    OrderEntry* curr = order_map_[h];
    OrderEntry* prev = nullptr;

    while (curr) {
        if (curr->id == id) {
            // Found it
            
            // 1. Update Levels
            update_level(curr->side, curr->price, -static_cast<int32_t>(curr->quantity));

            // 2. Unlink from Map
            if (prev) {
                prev->next = curr->next;
            } else {
                order_map_[h] = curr->next;
            }

            // 3. Return to Pool
            order_pool_.deallocate(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
}

// Optimized Level Update
// - Finds level using linear scan (fast for small MAX_LEVELS=100)
// - Inserts/Removes shifting memory with memmove
void OrderBookL2::update_level(Side side, Price price, int32_t qty_delta) noexcept {
    auto& levels = (side == Side::BUY) ? bids_ : asks_;
    
    // 1. Find the level
    for (size_t i = 0; i < MAX_LEVELS; ++i) {
        // Case A: Found existing level
        if (levels[i].price == price) {
            levels[i].quantity += qty_delta;
            if (qty_delta > 0) levels[i].order_count++;
            else levels[i].order_count--; // Approximation (doesn't handle multi-order-per-level perfectly without tracking)
            
            // If quantity drops to 0 (or less), remove level
            if (levels[i].quantity <= 0) {
                // Shift remaining levels left
                if (i < MAX_LEVELS - 1) {
                    std::memmove(&levels[i], &levels[i+1], (MAX_LEVELS - 1 - i) * sizeof(Level));
                }
                // Clear last
                levels[MAX_LEVELS-1].price = (side == Side::BUY) ? 0 : INVALID_PRICE;
                levels[MAX_LEVELS-1].quantity = 0;
            }
            return;
        }

        // Case B: Found insertion point (Empty slot OR correct sort order)
        // BUY: Descending (Current < Price)
        // SELL: Ascending (Current > Price)
        bool empty_slot = (side == Side::BUY && levels[i].price == 0) || 
                          (side == Side::SELL && levels[i].price == INVALID_PRICE);
        
        bool correct_order = (side == Side::BUY && levels[i].price < price) ||
                             (side == Side::SELL && levels[i].price > price);

        if (empty_slot || correct_order) {
            // If we are removing, we shouldn't be here (means price wasn't found)
            if (qty_delta < 0) return; 

            // Insert New Level
            // Shift right to make space
            if (i < MAX_LEVELS - 1) {
                std::memmove(&levels[i+1], &levels[i], (MAX_LEVELS - 1 - i) * sizeof(Level));
            }
            levels[i].price = price;
            levels[i].quantity = qty_delta;
            levels[i].order_count = 1;
            return;
        }
    }
}

} // namespace ultra::md
