#pragma once
#include "../strategy/strategy.hpp"
#include "execution_report.hpp"
#include <vector>
#include <algorithm>
#include <deque>

namespace ultra::exec {

/**
 * Exchange Matching Engine Simulator (Price-Time Priority)
 * Matches incoming orders against a resting book.
 */
class MatchingEngine {
public:
    struct BookOrder {
        strategy::StrategyOrder order;
        uint64_t timestamp; // For Time priority
    };

    // Output queue for execution reports
    std::deque<ExecutionReport> fills_;

    void process_order(const strategy::StrategyOrder& order) {
        // 1. Validate
        if (order.quantity <= 0) return;

        // 2. Try Match
        Quantity remaining_qty = order.quantity;
        
        if (order.side == Side::BUY) {
            match_buy(order, remaining_qty);
        } else {
            match_sell(order, remaining_qty);
        }

        // 3. Rest if not fully filled and is LIMIT
        if (remaining_qty > 0 && order.type == OrderType::LIMIT) {
            add_to_book(order, remaining_qty);
        }
    }

    bool has_fills() const { return !fills_.empty(); }
    
    bool pop_fill(ExecutionReport& fill) {
        if (fills_.empty()) return false;
        fill = fills_.front();
        fills_.pop_front();
        return true;
    }

private:
    std::vector<BookOrder> bids_; // Sorted Descending Price, then Time
    std::vector<BookOrder> asks_; // Sorted Ascending Price, then Time
    uint64_t sequence_{0};

    void match_buy(const strategy::StrategyOrder& incoming, Quantity& remaining) {
        while (remaining > 0 && !asks_.empty()) {
            BookOrder& best_ask = asks_.front();
            
            // Check Price
            if (incoming.price < best_ask.order.price) break; // No match
            
            // Match
            Quantity fill_qty = std::min(remaining, best_ask.order.quantity);
            
            generate_fill(incoming, best_ask.order, fill_qty, best_ask.order.price);
            
            remaining -= fill_qty;
            best_ask.order.quantity -= fill_qty;
            
            // Remove filled ask
            if (best_ask.order.quantity == 0) {
                asks_.erase(asks_.begin());
            }
        }
    }

    void match_sell(const strategy::StrategyOrder& incoming, Quantity& remaining) {
        while (remaining > 0 && !bids_.empty()) {
            BookOrder& best_bid = bids_.front();
            
            // Check Price
            if (incoming.price > best_bid.order.price) break; // No match
            
            // Match
            Quantity fill_qty = std::min(remaining, best_bid.order.quantity);
            
            generate_fill(incoming, best_bid.order, fill_qty, best_bid.order.price);
            
            remaining -= fill_qty;
            best_bid.order.quantity -= fill_qty;
            
            // Remove filled bid
            if (best_bid.order.quantity == 0) {
                bids_.erase(bids_.begin());
            }
        }
    }

    void add_to_book(const strategy::StrategyOrder& order, Quantity qty) {
        BookOrder book_order{order, ++sequence_};
        book_order.order.quantity = qty;

        if (order.side == Side::BUY) {
            bids_.push_back(book_order);
            // Sort: High Price First, then Low Time
            std::sort(bids_.begin(), bids_.end(), [](const BookOrder& a, const BookOrder& b) {
                if (a.order.price != b.order.price) return a.order.price > b.order.price;
                return a.timestamp < b.timestamp;
            });
        } else {
            asks_.push_back(book_order);
            // Sort: Low Price First, then Low Time
            std::sort(asks_.begin(), asks_.end(), [](const BookOrder& a, const BookOrder& b) {
                if (a.order.price != b.order.price) return a.order.price < b.order.price;
                return a.timestamp < b.timestamp;
            });
        }
    }

    void generate_fill(const strategy::StrategyOrder& aggressive, 
                       const strategy::StrategyOrder& passive, 
                       Quantity qty, Price price) {
        (void)passive; // Silence unused parameter warning

        // Fill for aggressive order
        fills_.push_back({
            .tsc = 0,
            .order_id = aggressive.order_id,
            .symbol_id = aggressive.symbol_id,
            .status = (aggressive.quantity == qty) ? OrderStatus::FILLED : OrderStatus::PARTIAL,
            .fill_price = price,
            .fill_quantity = qty,
            .remaining_quantity = aggressive.quantity - qty // Approximation for this event
        });
        
        // Fill for passive order (Counterparty)
        // In a real system, we'd send this to the other participant
        // For now, we simulate "Market Impact" effectively
    }
};

} // namespace ultra::exec
