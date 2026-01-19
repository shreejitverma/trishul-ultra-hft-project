#pragma once
#include "../core/types.hpp"

namespace ultra::exec {

// Exchange -> Engine
struct ExecutionReport {
    Timestamp tsc;
    OrderId order_id;
    SymbolId symbol_id;
    OrderStatus status;
    Price fill_price{0};
    Quantity fill_quantity{0};
    Quantity remaining_quantity{0};
};

} // namespace ultra::exec
