#include "ultra/strategy/market_maker.hpp"
#include <iostream>
#include <cassert>

using namespace ultra;
using namespace ultra::strategy;
using namespace ultra::md::itch;

int main() {
    std::cout << "[Test] Starting MarketMaker Strategy Test...\n";

    MarketMaker mm(1); // Symbol 1

    // 1. Send Market Data (Establish BBO)
    // Bid @ 100.00, Ask @ 101.00 -> Mid 100.50
    // Strategy should quote Buy @ 100.00, Sell @ 101.00 (if spread capture is 0.05)
    // Actually SPREAD_CAPTURE is 500 (0.05 in 4 decimal).
    // Let's use prices: 100000 (10.00), 100200 (10.02) -> Mid 100100.
    // Buy @ 100100 - 500 = 99600.
    // Sell @ 100100 + 500 = 100600.
    
    std::cout << "  -> Injecting Market Data...\n";
    
    ITCHDecoder::DecodedMessage msg_bid{};
    msg_bid.valid = true;
    msg_bid.symbol_id = 1;
    msg_bid.event_type = MDEventType::ADD_ORDER;
    msg_bid.order_id = 1;
    msg_bid.side = Side::BUY;
    msg_bid.price = 100000;
    msg_bid.quantity = 100;
    
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
    }

    if (order_count < 2) {
        std::cerr << "[FAIL] Expected at least 2 orders (Buy+Sell), got " << order_count << "\n";
        return 1;
    }

    std::cout << "[Test] MarketMaker Test Passed.\n";
    return 0;
}
