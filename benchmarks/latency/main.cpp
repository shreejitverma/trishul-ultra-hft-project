#include "ultra/market-data/itch/decoder.hpp"
#include "ultra/market-data/book/order_book_l2.hpp"
#include "ultra/core/time/rdtsc_clock.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace ultra;

template<typename Func>
std::vector<uint64_t> measure(const std::string& name, int iterations, Func&& f) {
    (void)name;
    std::vector<uint64_t> samples;
    samples.reserve(iterations);
    
    // Warmup
    for(int i=0; i<100; ++i) f();

    for(int i=0; i<iterations; ++i) {
        uint64_t start = RDTSCClock::rdtsc();
        f();
        uint64_t end = RDTSCClock::rdtsc();
        samples.push_back(end - start);
    }
    
    // Convert to ns (approx, assuming 1 tick = 1 ns for simplicity or calibrating)
    // We'll report in ticks to be pure.
    return samples;
}

void print_stats(const std::string& name, std::vector<uint64_t>& samples) {
    std::sort(samples.begin(), samples.end());
    double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
    double avg = sum / samples.size();
    uint64_t min = samples.front();
    uint64_t max = samples.back();
    uint64_t p50 = samples[samples.size() * 0.50];
    uint64_t p99 = samples[samples.size() * 0.99];
    
    std::cout << "[" << name << "] Ticks (Cycles):" << std::endl;
    std::cout << "  Min: " << min << std::endl;
    std::cout << "  Avg: " << avg << std::endl;
    std::cout << "  P50: " << p50 << std::endl;
    std::cout << "  P99: " << p99 << std::endl;
    std::cout << "  Max: " << max << std::endl;
}

int main() {
    RDTSCClock::calibrate();

    // 1. Measure RDTSC overhead
    auto rdtsc_samples = measure("RDTSC Overhead", 1000000, [](){
        auto t = RDTSCClock::rdtsc();
        (void)t;
    });
    print_stats("RDTSC", rdtsc_samples);

    // 2. Measure Decoder
    md::itch::AddOrder msg;
    msg.header.type = 'A';
    msg.header.length = __builtin_bswap16(sizeof(msg));
    msg.timestamp = 0;
    msg.order_ref_number = 1;
    msg.buy_sell_indicator = 'B';
    msg.shares = 100;
    memcpy(msg.stock, "AAPL    ", 8);
    msg.price = 100000;
    
    md::itch::ITCHDecoder decoder;
    decoder.register_symbol("AAPL    ", 1);
    
    auto decode_samples = measure("ITCH Decode (AddOrder)", 100000, [&](){
        auto res = decoder.decode(reinterpret_cast<uint8_t*>(&msg), sizeof(msg), 0);
        (void)res;
    });
    print_stats("Decode", decode_samples);

    // 3. Measure Book Update
    md::OrderBookL2 book(1);
    auto res = decoder.decode(reinterpret_cast<uint8_t*>(&msg), sizeof(msg), 0);
    
    auto book_samples = measure("Book Update (Add)", 100000, [&](){
        book.update(res); // Note: this will keep adding orders, book grows indefinitely
    });
    print_stats("BookUpdate", book_samples);

    return 0;
}
