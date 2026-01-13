#include "ultra/market-data/itch/decoder.hpp"
#include "ultra/market-data/book/order_book_l2.hpp"
#include "ultra/strategy/rl-inference/rl_policy.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <deque>
#include <iomanip>

using namespace ultra;

// ==============================================================================================
// Backtest Exchange Simulator
// ==============================================================================================
class BacktestExchange {
public:
    struct ActiveOrder {
        OrderId id;
        Side side;
        Price price;
        Quantity quantity;
        uint64_t entered_ts;
    };

    void place_order(const strategy::StrategyOrder& order, uint64_t now_ts) {
        // Simple OCO-like simulation: New order cancels old for this side
        // (Matches the strategy's behavior of constantly updating quotes)
        if (order.side == Side::BUY) {
            active_bid_ = {order.order_id, Side::BUY, order.price, order.quantity, now_ts};
            has_bid_ = true;
        } else {
            active_ask_ = {order.order_id, Side::SELL, order.price, order.quantity, now_ts};
            has_ask_ = true;
        }
    }

    void process_market_update(const md::OrderBookL2& book, uint64_t now_ts) {
        (void)now_ts;
        // Check for fills against the *current* market book
        
        // 1. Check Bid Fill (We are buying)
        // If our bid >= Best Ask, we cross spread -> Fill
        if (has_bid_) {
            auto best_ask = book.best_ask();
            if (best_ask.price != INVALID_PRICE && active_bid_.price >= best_ask.price) {
                // FILLED
                execute(active_bid_, best_ask.price);
                has_bid_ = false;
            }
        }

        // 2. Check Ask Fill (We are selling)
        // If our ask <= Best Bid, we cross spread -> Fill
        if (has_ask_) {
            auto best_bid = book.best_bid();
            if (best_bid.price != 0 && active_ask_.price <= best_bid.price) {
                // FILLED
                execute(active_ask_, best_bid.price);
                has_ask_ = false;
            }
        }
    }

    double get_pnl() const { return realized_pnl_; }
    Quantity get_position() const { return position_; }
    size_t get_trades() const { return trade_count_; }

private:
    bool has_bid_{false};
    ActiveOrder active_bid_;
    
    bool has_ask_{false};
    ActiveOrder active_ask_;

    Quantity position_{0};
    double realized_pnl_{0.0};
    double cash_{0.0};
    size_t trade_count_{0};

    void execute(const ActiveOrder& order, Price fill_price) {
        trade_count_++;
        double fill_price_dbl = from_price(fill_price);
        
        if (order.side == Side::BUY) {
            position_ += order.quantity;
            cash_ -= (fill_price_dbl * order.quantity);
        } else {
            position_ -= order.quantity;
            cash_ += (fill_price_dbl * order.quantity);
        }
        
        // Mark to market PnL approximation
        // (Realized is complex with FIFO/LIFO, here we use Cash + Position * LastPrice)
        realized_pnl_ = cash_ + (position_ * fill_price_dbl);
    }
};

// ==============================================================================================
// Main Backtester
// ==============================================================================================
int main(int argc, char** argv) {
    std::string filename = "market_data.bin";
    if (argc > 1) filename = argv[1];

    // 1. Setup
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open " << filename << ". Run data_generator first." << std::endl;
        return 1;
    }

    const SymbolId SYMBOL = 1;
    md::itch::ITCHDecoder decoder;
    decoder.register_symbol("AAPL    ", SYMBOL);
    
    // Exchange's Book (Truth)
    md::OrderBookL2 exchange_book(SYMBOL);

    strategy::RLPolicyStrategy strategy(SYMBOL);
    BacktestExchange exchange;

    // Buffer for reading messages
    std::vector<uint8_t> buffer(65536); // 64KB buffer
    size_t msg_count = 0;

    std::cout << "Starting Backtest on " << filename << "..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();

    while (in) {
        // Peek at header to get length
        uint16_t msg_len_be;
        if (!in.read(reinterpret_cast<char*>(&msg_len_be), 2)) break;
        
        // Seek back 2 bytes
        in.seekg(-2, std::ios::cur);
        
        uint16_t msg_len = __builtin_bswap16(msg_len_be);
        
        // Read full message
        in.read(reinterpret_cast<char*>(buffer.data()), msg_len);
        if (!in) break;

        // 1. Decode
        auto decoded = decoder.decode(buffer.data(), msg_len, 0); // TS doesn't matter for backtest logic here
        
        if (decoded.valid && decoded.symbol_id == SYMBOL) {
            // 2. Update Exchange Book
            exchange_book.update(decoded);

            // 3. Feed Strategy
            strategy.on_market_data(decoded);
            
            // 4. Get Orders from Strategy
            strategy::StrategyOrder order;
            while (strategy.get_order(order)) {
                exchange.place_order(order, decoded.exchange_ts);
            }

            // 5. Match Orders against new Book State
            exchange.process_market_update(exchange_book, decoded.exchange_ts);
        }
        
        msg_count++;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);

    // Report
    std::cout << "========================================" << std::endl;
    std::cout << "BACKTEST REPORT" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Messages Processed: " << msg_count << std::endl;
    std::cout << "Time Taken        : " << duration.count() << " ms" << std::endl;
    std::cout << "Throughput        : " << (msg_count * 1000.0 / duration.count()) << " msgs/sec" << std::endl;
    std::cout << "----------------------------------------" << std::endl;
    std::cout << "Total Trades      : " << exchange.get_trades() << std::endl;
    std::cout << "Final Position    : " << exchange.get_position() << std::endl;
    std::cout << "PnL (Mark-to-Mkt) : $" << std::fixed << std::setprecision(2) << exchange.get_pnl() << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
