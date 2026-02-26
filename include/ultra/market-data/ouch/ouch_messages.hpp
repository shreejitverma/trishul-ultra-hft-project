#pragma once
#include <cstdint>
#include <array>

namespace ultra::ouch {

#pragma pack(push, 1)

struct EnterOrder {
    char message_type{'O'}; ///< char variable representing message_type.
    uint64_t order_token; // User defined ID
    char buy_sell_indicator; // 'B' = Buy, 'S' = Sell
    uint32_t shares; ///< int variable representing shares.
    char stock[8]; ///< char[8] variable representing stock.
    uint32_t price; // 4 decimals
    uint32_t time_in_force{0}; // 0 = Day
    char firm[4]; ///< char[4] variable representing firm.
    char display{'Y'}; ///< char variable representing display.
    char capacity{'P'}; // Principal
    char iso_eligibility{'N'}; ///< char variable representing iso_eligibility.
    uint32_t min_qty{0}; ///< int variable representing min_qty.
    char cross_type{'N'}; ///< char variable representing cross_type.
    char customer_type{'R'}; // Retail
};

struct ReplaceOrder {
    char message_type{'U'}; ///< char variable representing message_type.
    uint64_t original_order_token; ///< int variable representing original_order_token.
    uint64_t new_order_token; ///< int variable representing new_order_token.
    uint32_t shares; ///< int variable representing shares.
    uint32_t price; ///< int variable representing price.
    uint32_t time_in_force{0}; ///< int variable representing time_in_force.
    char display{'Y'}; ///< char variable representing display.
    char iso_eligibility{'N'}; ///< char variable representing iso_eligibility.
    uint32_t min_qty{0}; ///< int variable representing min_qty.
};

struct CancelOrder {
    char message_type{'X'}; ///< char variable representing message_type.
    uint64_t order_token; ///< int variable representing order_token.
    uint32_t shares; // 0 = All
};

struct ExecutedOrder {
    char message_type{'E'}; ///< char variable representing message_type.
    uint64_t timestamp; ///< int variable representing timestamp.
    uint64_t order_token; ///< int variable representing order_token.
    uint32_t executed_shares; ///< int variable representing executed_shares.
    uint32_t execution_price; ///< int variable representing execution_price.
    uint64_t liquidity_flag; // Match ID
};

struct SystemEvent {
    char message_type{'S'}; ///< char variable representing message_type.
    uint64_t timestamp; ///< int variable representing timestamp.
    char event_code; // 'S' = Start, 'E' = End
};

#pragma pack(pop)

} // namespace ultra::ouch
