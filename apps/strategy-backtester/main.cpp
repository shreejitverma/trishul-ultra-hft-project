#include "ultra/market-data/itch/decoder.hpp"
#include "ultra/market-data/book/order_book_l2.hpp"
#include "ultra/strategy/signal_engine.hpp"
#include "ultra/strategy/performance_metrics.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

using namespace ultra;

// Simple structure to hold loaded tick data
struct TickData {
    std::vector<double> prices; ///< int variable representing prices.
    std::vector<uint64_t> timestamps; ///< int variable representing timestamps.
};

// Load Binary Data into Columnar format (Vectorized Friendly)
TickData load_data_vectorized(const std::string& filename, size_t max_limit = 0) {
    TickData data;
    std::ifstream in(filename, std::ios::binary);
    if (!in) return data;

    std::vector<uint8_t> buffer(256); // Small buffer for single messages
    size_t count = 0;
    
    // We only care about executions for price history in this simplified vector backtest
    // Or BBO updates. Let's use Executions as "Last Price"
    
    while (in) {
        uint16_t msg_len_be;
        if (!in.read(reinterpret_cast<char*>(&msg_len_be), 2)) break;
        in.seekg(-2, std::ios::cur);
        uint16_t msg_len = __builtin_bswap16(msg_len_be);
        
        if (buffer.size() < msg_len) buffer.resize(msg_len);
        in.read(reinterpret_cast<char*>(buffer.data()), msg_len);
        
        // Manual decode minimal
        uint8_t type = buffer[2]; // Offset 2 is Type
        if (type == 'E' || type == 'C') { // Executed
            // Extract Price
            // Header(3) + Ref(8) + ... 
            // See decoder.hpp for offsets. Or just use decoder
            // Let's assume OrderExecuted struct layout:
            // Hdr(3) + Loc(2) + Trk(2) + Time(6) + Ref(8) + Shares(4) + Match(8)
            // Wait, executed message 'E' doesn't have price in ITCH 5.0 (it uses Ref order price)
            // 'C' (Executed with Price) has price.
            // Our generator produces 'E'. We need to track order books to know price.
            // For this VECTOR example, let's assume we extract BBO mid-price from 'A' (Add Order) messages 
            // just to build a price series.
        } 
        else if (type == 'A' || type == 'F') { // Add Order
            // Use Add Order price as a proxy for market price updates
            // Struct AddOrder: Hdr(3) + Loc(2) + Trk(2) + Time(6) + Ref(8) + Side(1) + Shares(4) + Stock(8) + Price(4)
            // Offset of Price = 3+2+2+6+8+1+4+8 = 34
            if (msg_len >= 38) {
                uint32_t price_be = *reinterpret_cast<uint32_t*>(&buffer[34]);
                double price = static_cast<double>(__builtin_bswap32(price_be)) / 10000.0;
                data.prices.push_back(price);
                // Timestamp offset 7, 6 bytes. 
                // Simplified:
                data.timestamps.push_back(count); 
            }
        }

        count++;
        if (max_limit > 0 && count >= max_limit) break;
    }
    return data;
}

    /**
     * @brief Auto-generated description for main.
     * @param argc Parameter description.
     * @param argv Parameter description.
     * @return int value.
     */
int main(int argc, char** argv) {
    std::string filename = "market_data_large.bin";
    if (argc > 1) filename = argv[1];

    std::cout << "1. Loading Data (Vectorized)..." << std::endl;
    auto t1 = std::chrono::high_resolution_clock::now();
    
    TickData data = load_data_vectorized(filename, 10000000); // Try to load all
    
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "   Loaded " << data.prices.size() << " price points in " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;

    if (data.prices.empty()) {
        std::cerr << "No price data found. Run data_generator first." << std::endl;
        return 1;
    }

    std::cout << "2. Computing Signals (SIMD Optimized)..." << std::endl;
    auto t3 = std::chrono::high_resolution_clock::now();
    
    // Strategy: Moving Average Crossover + RSI Filter
    // 1. Compute Fast MA (50 period)
    auto ma_fast = strategy::SignalEngine::sma(data.prices, 50);
    
    // 2. Compute Slow MA (200 period)
    auto ma_slow = strategy::SignalEngine::sma(data.prices, 200);
    
    // 3. Compute RSI (14 period)
    auto rsi = strategy::SignalEngine::rsi(data.prices, 14);

    auto t4 = std::chrono::high_resolution_clock::now();
    std::cout << "   Signals Computed in " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count() << "ms" << std::endl;

    std::cout << "3. Simulating Execution (Vectorized)..." << std::endl;
    
    // Generate Signals
    // 1 = Buy, -1 = Sell
    auto signals = strategy::SignalEngine::ma_crossover_signal(ma_fast, ma_slow);
    
    // Apply RSI Filter (Don't buy if overbought > 70, Don't sell if oversold < 30)
    for (size_t i = 0; i < signals.size(); ++i) {
        if (signals[i] == 1 && rsi[i] > 70) signals[i] = 0;
        if (signals[i] == -1 && rsi[i] < 30) signals[i] = 0;
    }

    // Calculate Returns
    std::vector<double> equity_curve;
    equity_curve.reserve(data.prices.size());
    double cash = 100000.0;
    double holdings = 0;
    double equity = cash;
    
    for (size_t i = 0; i < signals.size(); ++i) {
        if (signals[i] == 1) { // Buy
            // Simple All-In model for vector test
            if (cash > 0) {
                holdings = cash / data.prices[i];
                cash = 0;
            }
        } else if (signals[i] == -1) { // Sell
            if (holdings > 0) {
                cash = holdings * data.prices[i];
                holdings = 0;
            }
        }
        
        equity = cash + (holdings * data.prices[i]);
        equity_curve.push_back(equity);
    }
    
    std::cout << "4. Analyzing Performance..." << std::endl;
    auto report = strategy::PerformanceAnalyst::analyze(equity_curve);
    report.print();

    return 0;
}