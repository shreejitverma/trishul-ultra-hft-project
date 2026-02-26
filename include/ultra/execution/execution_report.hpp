#pragma once
#include "../core/types.hpp"

namespace ultra::exec {

// Exchange -> Engine
struct ExecutionReport {
    Timestamp tsc; ///< int variable representing tsc.
    OrderId order_id; ///< int variable representing order_id.
    SymbolId symbol_id; ///< int variable representing symbol_id.
    OrderStatus status; ///< OrderStatus variable representing status.
    Price fill_price{0}; ///< int variable representing fill_price.
    Quantity fill_quantity{0}; ///< int variable representing fill_quantity.
    Quantity remaining_quantity{0}; ///< int variable representing remaining_quantity.
};

} // namespace ultra::exec
