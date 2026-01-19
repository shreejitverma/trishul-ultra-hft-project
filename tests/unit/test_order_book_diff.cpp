#include "ultra/market-data/book/order_book_l2.hpp"
#include "ultra/market-data/itch/decoder.hpp"
#include "ultra/core/types.hpp"
#include <iostream>
#include <cassert>

using namespace ultra;
using namespace ultra::md;
// ultra::md::itch is likely correct based on error
using namespace ultra::md::itch;

int main() {
    std::cout << "[Test] Starting OrderBook Diff Generator Test...\n";

    OrderBookL2 book(1); // Symbol ID 1
    
    bool callback_fired = false;
    uint64_t last_ts = 0;
    Price last_bid = 0;

    book.set_bbo_listener([&](const OrderBookL2::BBOUpdate& update) {
        callback_fired = true;
        last_ts = update.timestamp;
        last_bid = update.bid_price;
        std::cout << "  -> BBO Update: Bid=" << update.bid_price << " Ask=" << update.ask_price << "\n";
    });

    // 1. Add Order (Bid)
    std::cout << "[Test] Adding Bid 100x10 @ 100.00\n";
    ITCHDecoder::DecodedMessage msg{};
    msg.valid = true;
    msg.symbol_id = 1;
    msg.event_type = MDEventType::ADD_ORDER;
    msg.order_id = 101;
    msg.side = Side::BUY;
    msg.price = 10000; // 100.00
    msg.quantity = 10;
    
    book.update(msg);

    if (!callback_fired) {
        std::cerr << "[FAIL] Callback did not fire on first order!\n";
        return 1;
    }
    if (last_bid != 10000) {
        std::cerr << "[FAIL] Incorrect Bid Price!\n";
        return 1;
    }
    callback_fired = false;

    // 2. Add Better Bid (Update BBO)
    std::cout << "[Test] Adding Bid 100x10 @ 101.00 (New Top)\n";
    msg.order_id = 102;
    msg.price = 10100;
    book.update(msg);

    if (!callback_fired) {
        std::cerr << "[FAIL] Callback did not fire on better bid!\n";
        return 1;
    }
    if (last_bid != 10100) {
        std::cerr << "[FAIL] Incorrect Bid Price!\n";
        return 1;
    }
    callback_fired = false;

    // 3. Add Worse Bid (No BBO Change)
    std::cout << "[Test] Adding Bid 100x10 @ 99.00 (Deep)\n";
    msg.order_id = 103;
    msg.price = 9900;
    book.update(msg);

    if (callback_fired) {
        std::cerr << "[FAIL] Callback fired on deep book update!\n";
        return 1;
    }

    std::cout << "[Test] Passed All Checks.\n";
    return 0;
}
