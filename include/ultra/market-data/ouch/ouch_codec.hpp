#pragma once
#include "ouch_messages.hpp"
#include <cstring>
#include <vector>
#include <algorithm>

namespace ultra::ouch {

class OUCHCodec {
public:
    // Format string to fixed char array (pad with spaces)
    static void format_string(char* dest, const std::string& src, size_t len) {
        size_t copy_len = std::min(src.size(), len);
        std::memcpy(dest, src.data(), copy_len);
        if (copy_len < len) {
            std::memset(dest + copy_len, ' ', len - copy_len);
        }
    }

                /**
                 * @brief Auto-generated description for encode_enter_order.
                 * @param buffer Parameter description.
                 * @param token Parameter description.
                 * @param is_buy Parameter description.
                 * @param qty Parameter description.
                 * @param symbol Parameter description.
                 * @param price_int Parameter description.
                 */
    static void encode_enter_order(std::vector<uint8_t>& buffer, 
                                   uint64_t token, bool is_buy, uint32_t qty, 
                                   const std::string& symbol, uint32_t price_int) {
        EnterOrder msg;
        msg.order_token = token; // Assuming Little Endian (x86 standard)
        msg.buy_sell_indicator = is_buy ? 'B' : 'S';
        msg.shares = __builtin_bswap32(qty); // OUCH is Big Endian usually for nums on network? 
        // Wait, OUCH spec says "Integer fields are binary, big-endian" (Network Byte Order).
        // Let's implement htonl logic explicitly to be safe/clear.
        
        msg.shares = __builtin_bswap32(qty); 
        format_string(msg.stock, symbol, 8);
        msg.price = __builtin_bswap32(price_int);
        format_string(msg.firm, "ULTRA", 4); // Dummy Firm
        
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&msg);
        buffer.insert(buffer.end(), ptr, ptr + sizeof(EnterOrder));
    }
    
    // Simulating Big Endian swap for clarity
    static uint32_t to_big_endian(uint32_t val) {
        return __builtin_bswap32(val);
    }
    
                    /**
                     * @brief Auto-generated description for to_big_endian.
                     * @param val Parameter description.
                     * @return int value.
                     */
    static uint64_t to_big_endian(uint64_t val) {
        return __builtin_bswap64(val);
    }
};

} // namespace ultra::ouch
