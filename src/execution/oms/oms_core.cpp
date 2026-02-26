#include "ultra/execution/oms/oms_core.hpp"
#include <iostream>

namespace ultra::execution {

                            /**
                             * @brief Auto-generated description for send_order.
                             * @param order Parameter description.
                             */
void OrderManagementSystem::send_order(const StrategyOrder& order) {
    // 1. Assign ClOrdID (Client Order ID)
    auto cl_ord_id = next_cl_ord_id_++;
    
    // 2. Track Order State
    active_orders_[cl_ord_id] = OrderState{
        .cl_ord_id = cl_ord_id,
        .symbol_id = order.symbol_id,
        .price = order.price,
        .quantity = order.quantity,
        .side = order.side,
        .status = OrderStatus::NEW,
        .filled_qty = 0
    };
    
    std::cout << "[OMS] Sending New Order: ID=" << cl_ord_id 
              << " Side=" << (order.side == Side::BUY ? "BUY" : "SELL")
              << " Px=" << order.price << " Qty=" << order.quantity << std::endl;

    // 3. Forward to Router (Stub)
    // router_->route(active_orders_[cl_ord_id]);
}

void OrderManagementSystem::on_execution_report(const ExecutionReport& report) {
    auto it = active_orders_.find(report.cl_ord_id);
    if (it == active_orders_.end()) {
        std::cerr << "[OMS] ERR: Unknown Order ID " << report.cl_ord_id << std::endl;
        return;
    }
    
    OrderState& state = it->second;
    state.status = report.status;
    
    if (report.status == OrderStatus::FILLED || report.status == OrderStatus::PARTIAL) {
        state.filled_qty += report.fill_quantity;
        // Update PnL logic here
        std::cout << "[OMS] Fill Report: ID=" << report.cl_ord_id 
                  << " Filled=" << report.fill_quantity 
                  << " @ " << report.fill_price << std::endl;
    } else if (report.status == OrderStatus::CANCELED) {
        active_orders_.erase(it);
        std::cout << "[OMS] Order Canceled: ID=" << report.cl_ord_id << std::endl;
    }
}

} // namespace ultra::execution
