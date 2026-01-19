#pragma once
#include <cstdint>
#include <unordered_map>
#include "ultra/core/types.hpp"

namespace ultra::execution {

struct StrategyOrder {
    enum class Action { NEW_ORDER, CANCEL, REPLACE };
    Action action;
    uint64_t order_id; // Strategy-assigned ID
    SymbolId symbol_id;
    Side side;
    Price price;
    Quantity quantity;
    OrderType type;
};

enum class OrderStatus { NEW, PARTIAL, FILLED, CANCELED, REJECTED };

struct ExecutionReport {
    uint64_t cl_ord_id;
    SymbolId symbol_id;
    OrderStatus status;
    Quantity fill_quantity;
    Price fill_price;
};

struct OrderState {
    uint64_t cl_ord_id;
    SymbolId symbol_id;
    Price price;
    Quantity quantity;
    Side side;
    OrderStatus status;
    Quantity filled_qty;
};

class OrderManagementSystem {
public:
    OrderManagementSystem() = default;
    
    void send_order(const StrategyOrder& order);
    void on_execution_report(const ExecutionReport& report);
    
    // Telemetry
    size_t active_order_count() const { return active_orders_.size(); }

private:
    uint64_t next_cl_ord_id_ = 1;
    std::unordered_map<uint64_t, OrderState> active_orders_;
};

} // namespace ultra::execution
