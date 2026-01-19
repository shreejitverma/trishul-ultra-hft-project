#pragma once
#include <cstdint>
#include <array>

namespace ultra::ouch {

#pragma pack(push, 1)

struct EnterOrder {
    char message_type{'O'};
    uint64_t order_token; // User defined ID
    char buy_sell_indicator; // 'B' = Buy, 'S' = Sell
    uint32_t shares;
    char stock[8];
    uint32_t price; // 4 decimals
    uint32_t time_in_force{0}; // 0 = Day
    char firm[4];
    char display{'Y'};
    char capacity{'P'}; // Principal
    char iso_eligibility{'N'};
    uint32_t min_qty{0};
    char cross_type{'N'};
    char customer_type{'R'}; // Retail
};

struct ReplaceOrder {
    char message_type{'U'};
    uint64_t original_order_token;
    uint64_t new_order_token;
    uint32_t shares;
    uint32_t price;
    uint32_t time_in_force{0};
    char display{'Y'};
    char iso_eligibility{'N'};
    uint32_t min_qty{0};
};

struct CancelOrder {
    char message_type{'X'};
    uint64_t order_token;
    uint32_t shares; // 0 = All
};

struct ExecutedOrder {
    char message_type{'E'};
    uint64_t timestamp;
    uint64_t order_token;
    uint32_t executed_shares;
    uint32_t execution_price;
    uint64_t liquidity_flag; // Match ID
};

struct SystemEvent {
    char message_type{'S'};
    uint64_t timestamp;
    char event_code; // 'S' = Start, 'E' = End
};

#pragma pack(pop)

} // namespace ultra::ouch
