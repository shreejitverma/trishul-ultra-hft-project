#include "ultra/market-data/itch/decoder.hpp"
#include "ultra/market-data/book/order_book_l2.hpp"
#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>

using namespace ultra;

struct RawMessage {
    std::vector<uint8_t> data; ///< int variable representing data.
};

    /**
     * @brief Auto-generated description for main.
     * @param argc Parameter description.
     * @param argv Parameter description.
     * @return int value.
     */
int main(int argc, char** argv) {
    std::string filename = "market_data.bin";
    if (argc > 1) filename = argv[1];

    // 1. Pre-load data
    std::cout << "Loading " << filename << "..." << std::endl;
    std::ifstream in(filename, std::ios::binary);
    if (!in) {
        std::cerr << "Cannot open " << filename << std::endl;
        return 1;
    }

    std::vector<RawMessage> messages;
    messages.reserve(200000);

    while (in) {
        uint16_t msg_len_be;
        if (!in.read(reinterpret_cast<char*>(&msg_len_be), 2)) break;
        in.seekg(-2, std::ios::cur);
        uint16_t msg_len = __builtin_bswap16(msg_len_be);
        
        RawMessage msg;
        msg.data.resize(msg_len);
        in.read(reinterpret_cast<char*>(msg.data.data()), msg_len);
        if (in) messages.push_back(std::move(msg));
    }
    std::cout << "Loaded " << messages.size() << " messages." << std::endl;

    // 2. Setup
    const SymbolId SYMBOL = 1;
    md::itch::ITCHDecoder decoder;
    decoder.register_symbol("AAPL    ", SYMBOL);
    md::OrderBookL2 book(SYMBOL);

    // 3. Run Benchmark
    auto start = std::chrono::high_resolution_clock::now();
    
    // Warmup? No, let's just run.
    for (const auto& msg : messages) {
        auto decoded = decoder.decode(msg.data.data(), msg.data.size(), 0);
        if (decoded.valid) {
            book.update(decoded);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    double duration_sec = duration_ns / 1e9;

    std::cout << "Processed " << messages.size() << " messages in " << duration_sec << " s" << std::endl;
    std::cout << "Throughput: " << (messages.size() / duration_sec) / 1e6 << " M msgs/sec" << std::endl;
    std::cout << "Avg Latency: " << (duration_ns / messages.size()) << " ns/msg" << std::endl;

    return 0;
}
