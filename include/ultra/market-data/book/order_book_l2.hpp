#pragma once
#include "../../core/compiler.hpp"
#include "../../core/types.hpp"
#include "../../core/memory/object_pool.hpp"
#include "../itch/decoder.hpp"
#include <array>
#include <algorithm>
#include <vector>
#include <cstring>

namespace ultra::md {

/**
 * Optimized L2 Order Book
 * - Flat arrays for Price Levels (No std::map tree traversal)
 * - Open Addressing Hash Map for Orders (No std::unordered_map allocations)
 * - Object Pool for Order storage
 */
class OrderBookL2 {
public:
    static constexpr size_t MAX_LEVELS = 100; ///< const int variable representing MAX_LEVELS.
    static constexpr size_t MAX_ORDERS = 100000; // Power of 2 recommended for faster mod, but we use strict capacity
    static constexpr size_t HASH_SIZE = 131072; // Power of 2 > MAX_ORDERS for load factor < 0.8
    
    struct Level {
        Price price{INVALID_PRICE}; ///< int variable representing price.
        Quantity quantity{0}; ///< int variable representing quantity.
        uint32_t order_count{0}; ///< int variable representing order_count.
    };

    struct OrderEntry {
        OrderId id; ///< int variable representing id.
        Price price; ///< int variable representing price.
        Quantity quantity; ///< int variable representing quantity.
        Side side; ///< Side variable representing side.
        OrderEntry* next{nullptr}; // For collision chaining (simple for now, linear probing better but harder to delete)
    };

    // Bids sorted descending, Asks sorted ascending
    using PriceLevelSide = std::array<Level, MAX_LEVELS>;
    
    struct BBOUpdate {
        SymbolId symbol_id; ///< int variable representing symbol_id.
        Price bid_price; ///< int variable representing bid_price.
        Quantity bid_qty; ///< int variable representing bid_qty.
        Price ask_price; ///< int variable representing ask_price.
        Quantity ask_qty; ///< int variable representing ask_qty.
        uint64_t timestamp; // System time of update
    };

    using BBOListener = std::function<void(const BBOUpdate&)>;

    /**
     * @brief Auto-generated description for OrderBookL2.
     * @param symbol_id Parameter description.
     */
    OrderBookL2(SymbolId symbol_id);

         /**
          * @brief Auto-generated description for set_bbo_listener.
          * @param listener Parameter description.
          */
    void set_bbo_listener(BBOListener listener) { listener_ = std::move(listener); }

    // Apply a decoded ITCH message
    ULTRA_HOT void update(const itch::ITCHDecoder::DecodedMessage& msg) noexcept;

    // Get current BBO
    ULTRA_ALWAYS_INLINE const Level& best_bid() const noexcept { return bids_[0]; }
    ULTRA_ALWAYS_INLINE const Level& best_ask() const noexcept { return asks_[0]; }

    // Get all levels
    const PriceLevelSide& bids() const noexcept { return bids_; }
    const PriceLevelSide& asks() const noexcept { return asks_; }

private:
    SymbolId symbol_id_; ///< int variable representing symbol_id_.
    
    // --- 1. Order Storage (Object Pool) ---
    // Instead of allocating `L3Order` on heap, we use a pool.
    ObjectPool<OrderEntry, MAX_ORDERS> order_pool_; ///< int variable representing order_pool_.

    // --- 2. Order Lookup (Custom Hash Map) ---
    // Simple bucket array with chaining for this iteration. 
    // Ideally open-addressing, but chaining is safer for generic deletions without tombstones.
    std::array<OrderEntry*, HASH_SIZE> order_map_{}; // Init to nullptr

    // --- 3. Price Levels (Flat Arrays) ---
    ULTRA_CACHE_ALIGNED PriceLevelSide bids_{}; ///< int variable representing bids_.
    ULTRA_CACHE_ALIGNED PriceLevelSide asks_{}; ///< int variable representing asks_.
    
    BBOListener listener_; ///< int variable representing listener_.

    // Helpers
    ULTRA_ALWAYS_INLINE uint32_t hash(OrderId id) const {
        // Simple hash for sequential/dense IDs, FNV-1a better for random
        // ITCH Order IDs are 64-bit.
        id ^= id >> 33;
        id *= 0xff51afd7ed558ccd;
        id ^= id >> 33;
        id *= 0xc4ceb9fe1a85ec53;
        id ^= id >> 33;
        return id & (HASH_SIZE - 1);
    }
    
         /**
          * @brief Auto-generated description for add_order.
          * @param id Parameter description.
          * @param side Parameter description.
          * @param price Parameter description.
          * @param qty Parameter description.
          */
    void add_order(OrderId id, Side side, Price price, Quantity qty) noexcept;
         /**
          * @brief Auto-generated description for delete_order.
          * @param id Parameter description.
          */
    void delete_order(OrderId id) noexcept;
    
    // Update the flat L2 view when a level changes
    // This is the most expensive part if not careful.
    // For "Add", we often just increment qty if level exists.
    // For "Delete", we decrement. If 0, we shift array (memmove).
    // This approach avoids full rebuilds.
    void update_level(Side side, Price price, int32_t qty_delta) noexcept;
};

} // namespace ultra::md