#include "ultra/market-data/itch/decoder.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstring>
#include <chrono>

using namespace ultra;
using namespace ultra::md::itch;

// Helper to write struct to file
template<typename T>
void write_msg(std::ofstream& out, const T& msg) {
    out.write(reinterpret_cast<const char*>(&msg), sizeof(T));
}

uint64_t current_ts_ns() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

int main(int argc, char** argv) {
    std::string filename = "market_data.bin";
    size_t num_messages = 100000;

    if (argc > 1) filename = argv[1];
    if (argc > 2) num_messages = std::stoul(argv[2]);

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Generating " << num_messages << " messages to " << filename << "..." << std::endl;

    // Random Number Generation
    std::mt19937 rng(42); // Fixed seed for reproducibility
    std::normal_distribution<double> price_walk(0.0, 0.05); // Small price moves
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_int_distribution<uint32_t> qty_dist(1, 1000);
    
    // State
    double current_mid = 150.00;
    uint64_t order_ref = 1;
    uint64_t timestamp = 0; // Starts at midnight (0)

    // 1. Initial State (Add some resting orders)
    for (int i = 0; i < 20; ++i) {
        AddOrder msg;
        msg.header.type = static_cast<uint8_t>(MessageType::ADD_ORDER);
        msg.header.length = __builtin_bswap16(sizeof(AddOrder));
        msg.timestamp = __builtin_bswap64(timestamp++);
        msg.order_ref_number = __builtin_bswap64(order_ref++);
        msg.shares = __builtin_bswap32(100);
        std::memcpy(msg.stock, "AAPL    ", 8);

        // Spread around mid
        bool is_buy = side_dist(rng);
        double price = current_mid + (is_buy ? -1.0 : 1.0) * std::abs(price_walk(rng)) * 5.0; // Wider spread initially
        
        msg.buy_sell_indicator = is_buy ? 'B' : 'S';
        msg.price = __builtin_bswap32(static_cast<uint32_t>(price * 10000.0));

        write_msg(out, msg);
    }

    // 2. Simulation Loop
    for (size_t i = 0; i < num_messages; ++i) {
        // Random Walk
        current_mid += price_walk(rng);
        if (current_mid < 10.0) current_mid = 10.0; // Floor

        AddOrder msg;
        msg.header.type = static_cast<uint8_t>(MessageType::ADD_ORDER);
        msg.header.length = __builtin_bswap16(sizeof(AddOrder));
        msg.timestamp = __builtin_bswap64(timestamp += 100); // 100ns gap
        msg.order_ref_number = __builtin_bswap64(order_ref++);
        msg.shares = __builtin_bswap32(qty_dist(rng));
        std::memcpy(msg.stock, "AAPL    ", 8);

        bool is_buy = side_dist(rng);
        // Place orders near BBO
        double offset = std::abs(price_walk(rng)); // Always positive
        double price = current_mid + (is_buy ? -offset : offset);
        
        msg.buy_sell_indicator = is_buy ? 'B' : 'S';
        msg.price = __builtin_bswap32(static_cast<uint32_t>(price * 10000.0));

        write_msg(out, msg);

        // Occasional Trade (Execution)
        if (i % 10 == 0) {
            OrderExecuted exec_msg;
            exec_msg.header.type = static_cast<uint8_t>(MessageType::ORDER_EXECUTED);
            exec_msg.header.length = __builtin_bswap16(sizeof(OrderExecuted));
            exec_msg.timestamp = __builtin_bswap64(timestamp += 10);
            exec_msg.order_ref_number = __builtin_bswap64(order_ref - 1); // Execute the last order
            exec_msg.executed_shares = msg.shares; // Full fill
            exec_msg.match_number = __builtin_bswap64(i);
            
            write_msg(out, exec_msg);
        }
    }

    out.close();
    std::cout << "Done." << std::endl;
    return 0;
}
