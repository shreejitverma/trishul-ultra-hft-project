#pragma once
#include <cstdint>
#include <unordered_map>
#include "ultra/core/types.hpp"

namespace ultra::execution {

struct StrategyOrder {
    enum class Action { NEW_ORDER, CANCEL, REPLACE };
    Action action; ///< Action variable representing action.
    uint64_t order_id; // Strategy-assigned ID
    SymbolId symbol_id; ///< int variable representing symbol_id.
    Side side; ///< Side variable representing side.
    Price price; ///< int variable representing price.
    Quantity quantity; ///< int variable representing quantity.
    OrderType type; ///< OrderType variable representing type.
};

enum class OrderStatus { NEW, PARTIAL, FILLED, CANCELED, REJECTED };

struct ExecutionReport {
    uint64_t cl_ord_id; ///< int variable representing cl_ord_id.
    SymbolId symbol_id; ///< int variable representing symbol_id.
    OrderStatus status; ///< OrderStatus variable representing status.
    Quantity fill_quantity; ///< int variable representing fill_quantity.
    Price fill_price; ///< int variable representing fill_price.
};

struct OrderState {
    uint64_t cl_ord_id; ///< int variable representing cl_ord_id.
    SymbolId symbol_id; ///< int variable representing symbol_id.
    Price price; ///< int variable representing price.
    Quantity quantity; ///< int variable representing quantity.
    Side side; ///< Side variable representing side.
    OrderStatus status; ///< OrderStatus variable representing status.
    Quantity filled_qty; ///< int variable representing filled_qty.
};

class OrderManagementSystem {
public:
    /**
     * @brief Auto-generated description for OrderManagementSystem.
     */
    OrderManagementSystem() = default;
    
         /**
          * @brief Auto-generated description for send_order.
          * @param order Parameter description.
          */
    void send_order(const StrategyOrder& order);
         /**
          * @brief Auto-generated description for on_execution_report.
          * @param report Parameter description.
          */
    void on_execution_report(const ExecutionReport& report);
    
    // Telemetry
    size_t active_order_count() const { return active_orders_.size(); }

private:
    uint64_t next_cl_ord_id_ = 1; ///< int variable representing next_cl_ord_id_.
    std::unordered_map<uint64_t, OrderState> active_orders_; ///< int variable representing active_orders_.
};

} // namespace ultra::execution
