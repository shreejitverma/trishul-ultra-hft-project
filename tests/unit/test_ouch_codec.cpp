#include "ultra/market-data/ouch/ouch_codec.hpp"
#include <iostream>
#include <cassert>

using namespace ultra::ouch;

int main() {
    std::cout << "[Test] Starting OUCH Codec Test...\n";

    std::vector<uint8_t> buffer;
    OUCHCodec::encode_enter_order(buffer, 12345, true, 100, "AAPL", 1500000);

    // Verify Size
    if (buffer.size() != sizeof(EnterOrder)) {
        std::cerr << "[FAIL] Incorrect Packet Size. Got " << buffer.size() 
                  << " Expected " << sizeof(EnterOrder) << "\n";
        return 1;
    }

    // Verify Type
    if (buffer[0] != 'O') {
        std::cerr << "[FAIL] Incorrect Message Type.\n";
        return 1;
    }

    // Verify Side
    // offsetof(EnterOrder, buy_sell_indicator) = 1 + 8 = 9
    if (buffer[9] != 'B') {
        std::cerr << "[FAIL] Incorrect Side.\n";
        return 1;
    }

    std::cout << "[Test] OUCH Codec Test Passed.\n";
    return 0;
}
