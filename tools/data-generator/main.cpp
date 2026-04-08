#include "ultra/market-data/itch/decoder.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <cstring>
#include <chrono>
#include <cmath>
#include <queue>

using namespace ultra;
using namespace ultra::md::itch;

/**
 * @brief Realistic Market Data Generator using Hawkes Process for Event Arrivals.
 * 
 * In high-frequency regimes, event intensities are self-exciting. 
 * A Hawkes process captures this clustering:
 * Lambda(t) = Lambda_0 + Sum( Alpha * exp(-Beta * (t - t_i)) )
 */

// Helper to write struct to file
template<typename T>
void write_msg(std::ofstream& out, const T& msg) {
    out.write(reinterpret_cast<const char*>(&msg), sizeof(T));
}

int main(int argc, char** argv) {
    std::string filename = "market_data_hawkes.bin";
    size_t num_messages = 10000000; // 10 Million

    if (argc > 1) filename = argv[1];
    if (argc > 2) num_messages = std::stoul(argv[2]);

    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Failed to open output file: " << filename << std::endl;
        return 1;
    }

    std::cout << "Generating " << num_messages << " messages (Hawkes Process Simulation)..." << std::endl;

    // 1. Stochastic Price Params (GBM)
    double current_mid = 150.00;
    double mu = 0.0; 
    double sigma = 0.01;
    
    // 2. Hawkes Process Params (Microstructure Standards)
    double lambda_0 = 1000.0;  // Base intensity (events per second)
    double alpha = 0.6;        // Self-excitation factor (branching ratio)
    double beta = 2000.0;      // Decay rate
    double current_lambda = lambda_0;
    
    // RNG
    std::mt19937 rng(42);
    std::normal_distribution<double> norm_dist(0.0, 1.0);
    std::uniform_real_distribution<double> uni_dist(0.0, 1.0);
    std::uniform_int_distribution<uint32_t> size_dist(10, 1000);

    uint64_t order_ref = 1;
    double t_current = 0.0; // Current time in seconds
    uint64_t start_ns = 34200000000000ULL; // 09:30:00

    // Main Simulation Loop using Ogata's Thinning Algorithm for Hawkes Process
    for (size_t i = 0; i < num_messages; ++i) {
        // Step A: Generate next event time using thinning algorithm
        double lambda_max = current_lambda + lambda_0; // Upper bound for intensity
        double dt_event = 0.0;
        
        while (true) {
            double U1 = uni_dist(rng);
            dt_event += -std::log(U1) / lambda_max;
            
            // Recalculate intensity at prospective time t_current + dt_event
            double lambda_t = lambda_0 + (current_lambda - lambda_0) * std::exp(-beta * dt_event);
            
            double U2 = uni_dist(rng);
            if (U2 <= lambda_t / lambda_max) {
                // Accept event
                current_lambda = lambda_t + alpha * beta; // Pulse intensity on arrival
                break;
            }
            // Reject and continue (thinning)
            lambda_max = lambda_t + lambda_0;
        }

        t_current += dt_event;
        uint64_t timestamp_ns = start_ns + static_cast<uint64_t>(t_current * 1e9);

        // Step B: Update Price (GBM conditioned on event arrival)
        double dW = norm_dist(rng) * std::sqrt(dt_event);
        current_mid += current_mid * (mu * dt_event + sigma * dW);

        // Step C: Generate ITCH Message
        AddOrder msg;
        msg.header.type = 'A';
        msg.header.length = __builtin_bswap16(sizeof(AddOrder));
        msg.timestamp = __builtin_bswap64(timestamp_ns);
        msg.order_ref_number = __builtin_bswap64(order_ref++);
        msg.shares = __builtin_bswap32(size_dist(rng));
        memcpy(msg.stock, "AAPL    ", 8);

        bool is_buy = (uni_dist(rng) > 0.5);
        double spread = 0.01;
        double price = current_mid + (is_buy ? -spread/2 : spread/2);
        
        msg.buy_sell_indicator = is_buy ? 'B' : 'S';
        msg.price = __builtin_bswap32(static_cast<uint32_t>(price * 10000.0));
        write_msg(out, msg);

        // Progress log
        if (i % 1000000 == 0 && i > 0) {
            std::cout << "Hawkes Generation: " << i/1000000 << "M messages..." << std::endl;
        }
    }

    out.close();
    std::cout << "Done. Saved to " << filename << std::endl;
    return 0;
}
