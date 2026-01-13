#include "ultra/market-data/itch/decoder.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstring>
#include <chrono>
#include <cmath>

using namespace ultra;
using namespace ultra::md::itch;

// Helper to write struct to file
template<typename T>
void write_msg(std::ofstream& out, const T& msg) {
    out.write(reinterpret_cast<const char*>(&msg), sizeof(T));
}

int main(int argc, char** argv) {
    std::string filename = "market_data_large.bin";
    size_t num_messages = 10000000; // 10 Million

    if (argc > 1) filename = argv[1];
    if (argc > 2) num_messages = std::stoul(argv[2]);

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Generating " << num_messages << " messages (Realistic Simulation)..." << std::endl;

    // Stochastic Process Params (GBM + Jumps)
    double S0 = 150.00;
    double mu = 0.0001; // Slight drift
    double sigma = 0.02; // Volatility
    double dt = 1.0 / 23400.0; // 1 second in a trading day context
    
    // RNG
    std::mt19937 rng(42);
    std::normal_distribution<double> norm_dist(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> size_dist(10, 5000);
    std::uniform_real_distribution<double> jump_prob(0.0, 1.0);

    double current_mid = S0;
    uint64_t order_ref = 1;
    uint64_t timestamp = 34200000000000ULL; // 09:30:00 ns

    // 1. Initial Book Build
    for (int i = 0; i < 100; ++i) {
        AddOrder msg;
        msg.header.type = 'A';
        msg.header.length = __builtin_bswap16(sizeof(AddOrder));
        msg.timestamp = __builtin_bswap64(timestamp);
        msg.order_ref_number = __builtin_bswap64(order_ref++);
        msg.shares = __builtin_bswap32(100);
        memcpy(msg.stock, "AAPL    ", 8);

        bool is_buy = (i % 2 == 0);
        double price = current_mid + (is_buy ? -0.01 : 0.01) * ((i/2) + 1);
        
        msg.buy_sell_indicator = is_buy ? 'B' : 'S';
        msg.price = __builtin_bswap32(static_cast<uint32_t>(price * 10000.0));
        write_msg(out, msg);
    }

    // 2. Main Loop
    for (size_t i = 0; i < num_messages; ++i) {
        // Update Price (Geometric Brownian Motion)
        // dS = S * (mu*dt + sigma*dW)
        double dW = norm_dist(rng) * std::sqrt(dt);
        double dS = current_mid * (mu * dt + sigma * dW);
        
        // Add random jumps (fat tails)
        if (jump_prob(rng) < 0.001) { // 0.1% chance
            dS += (norm_dist(rng) * 0.5); 
        }

        current_mid += dS;
        if (current_mid < 10.0) current_mid = 10.0;

        timestamp += (std::rand() % 1000); // Random sub-microsecond gap

        // Generate Limit Order (Add)
        AddOrder msg;
        msg.header.type = 'A';
        msg.header.length = __builtin_bswap16(sizeof(AddOrder));
        msg.timestamp = __builtin_bswap64(timestamp);
        msg.order_ref_number = __builtin_bswap64(order_ref++);
        msg.shares = __builtin_bswap32(size_dist(rng));
        memcpy(msg.stock, "AAPL    ", 8);

        bool is_buy = (norm_dist(rng) > 0);
        // Place near mid
        double spread = 0.01 + std::abs(norm_dist(rng)) * 0.02;
        double price = current_mid + (is_buy ? -spread/2 : spread/2);
        
        msg.buy_sell_indicator = is_buy ? 'B' : 'S';
        msg.price = __builtin_bswap32(static_cast<uint32_t>(price * 10000.0));
        write_msg(out, msg);

        // Occasional Execution (Trade)
        if (i % 20 == 0) {
            OrderExecuted exec_msg;
            exec_msg.header.type = 'E';
            exec_msg.header.length = __builtin_bswap16(sizeof(OrderExecuted));
            exec_msg.timestamp = __builtin_bswap64(timestamp + 10);
            exec_msg.order_ref_number = __builtin_bswap64(order_ref - 1);
            exec_msg.executed_shares = msg.shares;
            exec_msg.match_number = __builtin_bswap64(i);
            write_msg(out, exec_msg);
        }
        
        // Progress log
        if (i % 1000000 == 0 && i > 0) {
            std::cout << "Generated " << i/1000000 << "M messages..." << std::endl;
        }
    }

    out.close();
    std::cout << "Done. Saved to " << filename << std::endl;
    return 0;
}