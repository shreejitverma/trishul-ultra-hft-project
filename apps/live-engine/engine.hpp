#pragma once
#include <ultra/core/types.hpp>
#include <ultra/market-data/itch/decoder.hpp>
#include <ultra/strategy/rl-inference/rl_policy.hpp>
#include <ultra/risk/pretrade_checker.hpp>
#include <ultra/execution/gateway_sim.hpp>
#include <ultra/execution/router/sor.hpp>
#include <ultra/network/multicast_receiver.hpp>
#include <ultra/fpga/fpga_driver.hpp>
#include <memory>
#include <thread>
#include <atomic>

namespace ultra {

/**
 * This is the main orchestrator, analogous to your thesis's
 * "Unified High-Frequency Trading System Architecture" (Fig 8) [cite: 250]
 */
class Engine {
public:
    Engine();
    /**
     * @brief Auto-generated description for ~Engine.
     */
    ~Engine();

         /**
          * @brief Auto-generated description for run.
          */
    void run();
         /**
          * @brief Auto-generated description for stop.
          */
    void stop();

private:
         /**
          * @brief Auto-generated description for md_thread_loop.
          */
    void md_thread_loop();    // Market Data Ingress
         /**
          * @brief Auto-generated description for exec_thread_loop.
          */
    void exec_thread_loop();  // Execution/OMS Loop
         /**
          * @brief Auto-generated description for strategy_thread_loop.
          */
    void strategy_thread_loop(); // Strategy Decision Loop

    // --- Components ---
    std::unique_ptr<md::itch::ITCHDecoder> decoder_; ///< int variable representing decoder_.
    std::unique_ptr<strategy::RLPolicyStrategy> strategy_; ///< int variable representing strategy_.
    std::unique_ptr<risk::PretradeChecker> risk_checker_; ///< int variable representing risk_checker_.
    std::unique_ptr<exec::GatewaySim> gateway_; ///< int variable representing gateway_.
    std::unique_ptr<execution::SmartOrderRouter> router_; ///< int variable representing router_.
    
    // New Components (Thesis Integration)
    std::unique_ptr<network::MulticastReceiver> udp_receiver_; ///< int variable representing udp_receiver_.
    std::unique_ptr<fpga::FPGADriver> fpga_driver_; ///< int variable representing fpga_driver_.
    
    bool use_live_network_{false}; // Set to true to use UDP Receiver
    
    // --- Message Queues (The "Event Driven Pipeline" from Fig 3) ---
    // (Using SPSC queues as this is a simple 1-to-1 pipeline)
    
    // MD -> Strategy
    using MDQueue = SPSCQueue<md::itch::ITCHDecoder::DecodedMessage, 16384>;
    std::unique_ptr<MDQueue> md_to_strategy_queue_; ///< int variable representing md_to_strategy_queue_.
    
    // Strategy -> Risk
    using OrderQueue = SPSCQueue<strategy::StrategyOrder, 8192>;
    std::unique_ptr<OrderQueue> strategy_to_risk_queue_; ///< int variable representing strategy_to_risk_queue_.

    // Risk -> Gateway
    std::unique_ptr<OrderQueue> risk_to_gateway_queue_; ///< int variable representing risk_to_gateway_queue_.

    // Gateway -> Strategy (Exec Reports)
    using ExecQueue = SPSCQueue<exec::ExecutionReport, 8192>;
    std::unique_ptr<ExecQueue> gateway_to_strategy_queue_; ///< int variable representing gateway_to_strategy_queue_.

    // --- Threads ---
    std::thread md_thread_; ///< int variable representing md_thread_.
    std::thread exec_thread_; ///< int variable representing exec_thread_.
    std::thread strategy_thread_; ///< int variable representing strategy_thread_.
    std::atomic<bool> running_{false}; ///< int variable representing running_.
};

} // namespace ultra
