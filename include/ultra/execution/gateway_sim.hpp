#pragma once
#include "../core/types.hpp"
#include "../core/lockfree/spsc_queue.hpp"
#include "../market-data/itch/decoder.hpp"
#include "execution_report.hpp"
#include <vector>

namespace ultra::strategy {
    struct StrategyOrder; // <-- ADDED FORWARD DECLARATION
}

namespace ultra::exec {

/**
 * A simple, single-threaded, simulated exchange gateway
 * It pretends to be an exchange, processing orders and sending
 * execution reports back.
 */
class GatewaySim {
public:
    GatewaySim() = default;

    // Engine -> Gateway
    void send_order(const strategy::StrategyOrder& order);

    // Gateway -> Engine
    bool get_execution_report(ExecutionReport& report);

    // Simulate market processing (call this in a loop)
    void update_market(const md::itch::ITCHDecoder::DecodedMessage& msg);

private:
    // A *very* simple matching engine
    // Bids (descending)
    std::vector<strategy::StrategyOrder> active_bids_;  ///< int variable representing active_bids_.
    // Asks (ascending)
    std::vector<strategy::StrategyOrder> active_asks_;  ///< int variable representing active_asks_.

    SPSCQueue<ExecutionReport, 8192> exec_reports_queue_; ///< int variable representing exec_reports_queue_.
    OrderId next_order_id_{1}; ///< int variable representing next_order_id_.

         /**
          * @brief Auto-generated description for try_match.
          * @param order Parameter description.
          */
    void try_match(strategy::StrategyOrder& order);
         /**
          * @brief Auto-generated description for send_fill.
          * @param order Parameter description.
          * @param fill_price Parameter description.
          * @param fill_qty Parameter description.
          */
    void send_fill(const strategy::StrategyOrder& order, Price fill_price, Quantity fill_qty);
         /**
          * @brief Auto-generated description for send_accept.
          * @param order Parameter description.
          */
    void send_accept(const strategy::StrategyOrder& order);
};

} // namespace ultra::exec