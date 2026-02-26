#include "engine.hpp"
#include "ultra/core/time/rdtsc_clock.hpp"
#include "ultra/core/thread_utils.hpp"
#include "ultra/core/async_logger.hpp"
#include <iostream>
#include <vector>

namespace ultra {

        /**
         * @brief Auto-generated description for Engine.
         */
Engine::Engine() {
    ULTRA_TRACE("Engine::Engine", "Constructing HFT Engine", "None");
    // --- 1. Allocate Queues ---
    md_to_strategy_queue_ = std::make_unique<MDQueue>();
    strategy_to_risk_queue_ = std::make_unique<OrderQueue>();
    risk_to_gateway_queue_ = std::make_unique<OrderQueue>();
    gateway_to_strategy_queue_ = std::make_unique<ExecQueue>();
    
    // --- 2. Initialize Components ---
    decoder_ = std::make_unique<md::itch::ITCHDecoder>();
    
    // Configure Symbol Universe (Hybrid Routing)
    const SymbolId AAPL_ID = 1;
    const SymbolId MSFT_ID = 2;
    auto& universe = SymbolUniverse::instance();
    
    // AAPL: High Frequency, FPGA Execution
    universe.add_symbol({AAPL_ID, "AAPL", 100, 1, -0.0002, 0.0003, true}); 
    // MSFT: Standard, CPU Execution
    universe.add_symbol({MSFT_ID, "MSFT", 100, 1, -0.0002, 0.0003, false});

    decoder_->register_symbol("AAPL    ", AAPL_ID);
    
    strategy_ = std::make_unique<strategy::RLPolicyStrategy>(AAPL_ID);
    
    risk::PretradeChecker::Config risk_config;
    risk_checker_ = std::make_unique<risk::PretradeChecker>(risk_config);
    
    gateway_ = std::make_unique<exec::GatewaySim>();

    // --- 3. Initialize New Thesis Components ---
    fpga_driver_ = std::make_unique<fpga::FPGADriver>();
    if (!fpga_driver_->init()) {
        std::cerr << "Warning: FPGA Driver Init Failed (Simulation Mode?)" << std::endl;
    }
    
    // Initialize Smart Order Router (Hybrid)
    router_ = std::make_unique<execution::SmartOrderRouter>(fpga_driver_.get(), gateway_.get());

    // Config for Live Network (Example)
    // In production, load from toml
    network::MulticastReceiver::Config net_config;
    net_config.multicast_group = "233.54.12.111"; // Example NASDAQ ITCH Group
    net_config.port = 5000;
    net_config.interface_ip = "127.0.0.1";
    udp_receiver_ = std::make_unique<network::MulticastReceiver>(net_config);
    
    // Default to Simulation for safety unless env var is set
    if (std::getenv("ULTRA_LIVE_MODE")) {
        use_live_network_ = true;
        if (!udp_receiver_->start()) {
            std::cerr << "Failed to start UDP receiver. Falling back to sim." << std::endl;
            use_live_network_ = false;
        }
    }
    
    std::cout << "Engine components initialized. Mode: " << (use_live_network_ ? "LIVE" : "SIMULATION") << std::endl;
}

        /**
         * @brief Auto-generated description for ~Engine.
         */
Engine::~Engine() {
    ULTRA_TRACE_SIMPLE("Engine::~Engine");
    if (running_) {
        stop();
    }
}

             /**
              * @brief Start all engine threads and enter running state.
              */
void Engine::run() {
    ULTRA_TRACE_SIMPLE("Engine::run");
    running_ = true;
    
    // Start threads (in reverse order of data flow)
    exec_thread_ = std::thread(&Engine::exec_thread_loop, this);
    strategy_thread_ = std::thread(&Engine::strategy_thread_loop, this);
    md_thread_ = std::thread(&Engine::md_thread_loop, this);
}

             /**
              * @brief Auto-generated description for stop.
              */
void Engine::stop() {
    ULTRA_TRACE_SIMPLE("Engine::stop");
    running_ = false;
    
    if (udp_receiver_) udp_receiver_->stop();
    
    if (md_thread_.joinable()) md_thread_.join();
    if (strategy_thread_.joinable()) strategy_thread_.join();
    if (exec_thread_.joinable()) exec_thread_.join();
    
    ULTRA_LOG_FLOW(INFO, "Info", 0, "Engine stopped.");
}

             /**
              * @brief Auto-generated description for md_thread_loop.
              */
void Engine::md_thread_loop() {
    ULTRA_TRACE_SIMPLE("Engine::md_thread_loop");
    ThreadUtils::pin_thread(1); // Pin MD to Core 1
    ULTRA_LOG_FLOW(INFO, "Info", 0, "[MD Thread] running.");
    
    // Buffer for network packets
    std::vector<uint8_t> rx_buffer(2048);

    // Simulation State
    using namespace md::itch;
    std::vector<uint8_t> sim_add_order(sizeof(AddOrder));
    auto* add_msg = reinterpret_cast<AddOrder*>(sim_add_order.data());
    // (Setup sim data...)
    add_msg->header.type = static_cast<uint8_t>(MessageType::ADD_ORDER);
    add_msg->header.length = __builtin_bswap16(sizeof(AddOrder));
    add_msg->shares = __builtin_bswap32(100);
    memcpy(add_msg->stock, "AAPL    ", 8);
    uint64_t order_ref_base = 10000;
    bool side_toggle = false;

    while (running_) {
        const uint8_t* packet_ptr = nullptr;
        size_t packet_len = 0;
        Timestamp rdtsc_ts = RDTSCClock::rdtsc();

        if (use_live_network_) {
            // --- LIVE PATH ---
            int n = udp_receiver_->receive(rx_buffer.data(), rx_buffer.size());
            if (n > 0) {
                packet_ptr = rx_buffer.data();
                packet_len = static_cast<size_t>(n);
            } else {
                // No data, spin or yield
                // std::this_thread::yield(); 
                continue;
            }
        } else {
            // --- SIMULATION PATH ---
            add_msg->timestamp = __builtin_bswap64(RDTSCClock::now());
            add_msg->order_ref_number = __builtin_bswap64(++order_ref_base);
            if (!side_toggle) {
                add_msg->buy_sell_indicator = 'B';
                add_msg->price = __builtin_bswap32(1500000); 
            } else {
                add_msg->buy_sell_indicator = 'S';
                add_msg->price = __builtin_bswap32(1500500); 
            }
            side_toggle = !side_toggle;
            
            packet_ptr = sim_add_order.data();
            packet_len = sim_add_order.size();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        
        // 2. Decode
        // Note: Real ITCH feed has multiple messages per UDP packet. 
        // A full implementation loops over the buffer parsing message lengths.
        // For this Thesis implementation, we assume 1 msg/packet or handle first.
        
        // Simple loop for multiple messages in buffer (if live)
        size_t offset = 0;
        while (offset + 2 <= packet_len) { // Need at least 2 bytes for length
             // ITCH 5.0 starts with Length (2 bytes)
             uint16_t msg_len_be = *reinterpret_cast<const uint16_t*>(packet_ptr + offset);
             uint16_t msg_len = __builtin_bswap16(msg_len_be);
             
             if (offset + 2 + msg_len > packet_len) break; // Incomplete
             
             // Decode payload (skip length bytes)
             auto decoded = decoder_->decode(packet_ptr + offset + 2, msg_len, rdtsc_ts);
             
             if (decoded.valid) {
                 if (ULTRA_UNLIKELY(!md_to_strategy_queue_->push(decoded))) {
                     // Drop
                 }
             }
             offset += 2 + msg_len;
             
             if (!use_live_network_) break; // Sim sends one struct, not ITCH framed packet
        }
    }
}

             /**
              * @brief Auto-generated description for strategy_thread_loop.
              */
void Engine::strategy_thread_loop() {
    ULTRA_TRACE_SIMPLE("Engine::strategy_thread_loop");
    ThreadUtils::pin_thread(2);
    ULTRA_LOG_FLOW(INFO, "Info", 0, "[Strategy Thread] running.");
    
    md::itch::ITCHDecoder::DecodedMessage md_msg;
    exec::ExecutionReport exec_report;
    strategy::StrategyOrder strategy_order;
    
    // Throttle FPGA updates
    int msg_counter = 0;

    while (running_) {
        bool work_done = false;
        
        if (md_to_strategy_queue_->pop(md_msg)) {
            strategy_->on_market_data(md_msg);
            work_done = true;
            msg_counter++;
        }
        
        if (gateway_to_strategy_queue_->pop(exec_report)) {
            strategy_->on_execution(exec_report);
            work_done = true;
        }

        while (strategy_->get_order(strategy_order)) {
            if (ULTRA_UNLIKELY(!strategy_to_risk_queue_->push(strategy_order))) {
                // Drop
            }
            work_done = true;
        }
        
        // Update FPGA Parameters periodically
        if (msg_counter >= 100) {
            fpga_driver_->update_strategy_params(0.1, 2.0, 1000); 
            msg_counter = 0;
        }
        
        if (!work_done) {
            // Spin
        }
    }
}

             /**
              * @brief Auto-generated description for exec_thread_loop.
              */
void Engine::exec_thread_loop() {
    ULTRA_TRACE_SIMPLE("Engine::exec_thread_loop");
    ThreadUtils::pin_thread(3);
    ULTRA_LOG_FLOW(INFO, "Info", 0, "[Exec Thread] running.");

    strategy::StrategyOrder order_to_check;
    exec::ExecutionReport exec_report;
    
    while (running_) {
        bool work_done = false;
        
        if (strategy_to_risk_queue_->pop(order_to_check)) {
            if (ULTRA_LIKELY(risk_checker_->check_order(order_to_check))) {
                // Route via Smart Order Router (FPGA vs CPU decision)
                router_->route(order_to_check);
            }
            work_done = true;
        }
        
        if (gateway_->get_execution_report(exec_report)) {
            if (ULTRA_UNLIKELY(!gateway_to_strategy_queue_->push(exec_report))) {
                // Drop
            }
            work_done = true;
        }
        
        if (!work_done) {
            // Spin
        }
    }
}

} // namespace ultra