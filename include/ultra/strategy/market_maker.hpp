#pragma once
#include "strategy.hpp"
#include "../market-data/book/order_book_l2.hpp"
#include "../core/lockfree/spsc_queue.hpp"
#include "../core/compiler.hpp"
#include "../risk/pretrade_checker.hpp"

namespace ultra::strategy {

/**
 * Basic Market Making Strategy (Liquidity Provider)
 * - Listens to BBO updates
 * - Maintains a 2-sided quote around the mid-price
 * - Captures the spread
 */
class MarketMaker : public IStrategy {
public:
    static constexpr size_t ORDER_QUEUE_CAPACITY = 1024; ///< const int variable representing ORDER_QUEUE_CAPACITY.

    /**
     * @brief Auto-generated description for MarketMaker.
     * @param symbol_id Parameter description.
     */
    MarketMaker(SymbolId symbol_id) 
        : symbol_id_(symbol_id), book_(symbol_id), 
          risk_checker_(risk::PretradeChecker::Config{}) { // Default risk limits
        
        // Setup BBO listener
        book_.set_bbo_listener([this](const md::OrderBookL2::BBOUpdate& bbo) {
            this->on_bbo_update(bbo);
        });
    }

         /**
          * @brief Auto-generated description for on_market_data.
          * @param msg Parameter description.
          */
    void on_market_data(const md::itch::ITCHDecoder::DecodedMessage& msg) override {
        // Feed the book, which triggers the listener
        book_.update(msg);
    }

         /**
          * @brief Auto-generated description for on_execution.
          * @param report Parameter description.
          */
    void on_execution(const exec::ExecutionReport& report) override {
        // Update Risk State
        risk_checker_.on_execution(report);
    }

         /**
          * @brief Auto-generated description for get_order.
          * @param order Parameter description.
          * @return bool value.
          */
    bool get_order(StrategyOrder& order) override {
        return order_queue_.pop(order);
    }

private:
    SymbolId symbol_id_; ///< int variable representing symbol_id_.
    md::OrderBookL2 book_; ///< md::OrderBookL2 variable representing book_.
    SPSCQueue<StrategyOrder, ORDER_QUEUE_CAPACITY> order_queue_; ///< int variable representing order_queue_.
    risk::PretradeChecker risk_checker_; ///< risk::PretradeChecker variable representing risk_checker_.
    
    // Strategy Parameters
    static constexpr Price SPREAD_CAPTURE = 500; // 5 cents
    static constexpr Quantity QUOTE_QTY = 100; ///< const int variable representing QUOTE_QTY.
    static constexpr double SKEW_FACTOR = 200.0; // 2 cents max skew
    
         /**
          * @brief Auto-generated description for on_bbo_update.
          * @param bbo Parameter description.
          */
    void on_bbo_update(const md::OrderBookL2::BBOUpdate& bbo) {
        // Simple logic: Quote around Mid Price
        if (bbo.bid_price == 0 || bbo.ask_price == INVALID_PRICE) return;

        Price mid_price = (bbo.bid_price + bbo.ask_price) / 2;
        
        // 1. Calculate Order Book Imbalance (OBI)
        // OBI = (BidQty - AskQty) / (BidQty + AskQty)
        // Range: [-1, 1]
        double total_qty = static_cast<double>(bbo.bid_qty + bbo.ask_qty);
        double obi = 0.0;
        if (total_qty > 0) {
            obi = static_cast<double>(bbo.bid_qty - bbo.ask_qty) / total_qty;
        }
        
        // 2. Skew Quotes based on OBI
        // Positive OBI (More Bids) -> Skew Up (Higher Bid, Higher Ask) to capture flow
        Price skew = static_cast<Price>(obi * SKEW_FACTOR);
        
        StrategyOrder buy_order;
        buy_order.action = StrategyOrder::NEW_ORDER;
        buy_order.symbol_id = symbol_id_;
        buy_order.side = Side::BUY;
        buy_order.price = mid_price - SPREAD_CAPTURE + skew;
        buy_order.quantity = QUOTE_QTY;
        buy_order.type = OrderType::LIMIT;
        
        StrategyOrder sell_order;
        sell_order.action = StrategyOrder::NEW_ORDER;
        sell_order.symbol_id = symbol_id_;
        sell_order.side = Side::SELL;
        sell_order.price = mid_price + SPREAD_CAPTURE + skew;
        sell_order.quantity = QUOTE_QTY;
        sell_order.type = OrderType::LIMIT;
        
        // Push orders (if space AND Risk OK)
        if (risk_checker_.check_order(buy_order)) {
            order_queue_.push(buy_order);
        }
        if (risk_checker_.check_order(sell_order)) {
            order_queue_.push(sell_order);
        }
    }
};

} // namespace ultra::strategy
