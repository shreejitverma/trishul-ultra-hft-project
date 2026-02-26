#include "ultra/strategy/market_maker.hpp"
#include <iostream>
#include <cassert>

using namespace ultra;
using namespace ultra::strategy;
using namespace ultra::md::itch;

    /**
     * @brief Auto-generated description for main.
     * @return int value.
     */
int main() {
    std::cout << "[Test] Starting MarketMaker Strategy Test...\n";

    MarketMaker mm(1); // Symbol 1

    // 1. Send Market Data (Establish BBO)
    // Bid @ 100.00 (Size 900), Ask @ 100.20 (Size 100) -> Mid 100.10
    // OBI = (900-100)/(900+100) = 800/1000 = 0.8
    // Skew = 0.8 * 200 = 160.
    // Buy Price = 100100 - 500 + 160 = 99760.
    // Sell Price = 100100 + 500 + 160 = 100760.
    
    std::cout << "  -> Injecting Market Data (Imbalanced)...\n";
    
    ITCHDecoder::DecodedMessage msg_bid{};
    msg_bid.valid = true;
    msg_bid.symbol_id = 1;
    msg_bid.event_type = MDEventType::ADD_ORDER;
    msg_bid.order_id = 1;
    msg_bid.side = Side::BUY;
    msg_bid.price = 100000;
    msg_bid.quantity = 900;
    
    ITCHDecoder::DecodedMessage msg_ask{};
    msg_ask.valid = true;
    msg_ask.symbol_id = 1;
    msg_ask.event_type = MDEventType::ADD_ORDER;
    msg_ask.order_id = 2;
    msg_ask.side = Side::SELL;
    msg_ask.price = 100200;
    msg_ask.quantity = 100;

    // Update MM
    mm.on_market_data(msg_bid);
    mm.on_market_data(msg_ask); // This triggers BBO change -> Logic

    // 2. Check Orders
    StrategyOrder order;
    int order_count = 0;
    while (mm.get_order(order)) {
        order_count++;
        std::cout << "  -> Generated Order: " 
                  << (order.side == Side::BUY ? "BUY" : "SELL") 
                  << " @ " << order.price << "\n";
        
        // Verify Skew
        if (order.side == Side::BUY) {
            // Unskewed would be 100100 - 500 = 99600.
            // We expect > 99600
            if (order.price <= 99600) {
                 std::cerr << "[FAIL] Buy Order Not Skewed Upwards! Price=" << order.price << "\n";
                 return 1;
            }
        }
    }

    if (order_count < 2) {
        std::cerr << "[FAIL] Expected at least 2 orders (Buy+Sell), got " << order_count << "\n";
        return 1;
    }

    std::cout << "[Test] MarketMaker Test Passed.\n";
    return 0;
}
