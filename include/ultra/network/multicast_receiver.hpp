#pragma once
#include "../core/compiler.hpp"
#include "../core/types.hpp"
#include <string>
#include <vector>
#include <atomic>
#include <netinet/in.h>

namespace ultra::network {

/**
 * High-Performance UDP Multicast Receiver
 * Supports:
 * - Multicast Group Joining
 * - SO_REUSEPORT/SO_REUSEADDR
 * - Non-blocking IO
 * - Kernel Bypass simulation (via optimized socket options)
 */
class MulticastReceiver {
public:
    struct Config {
        std::string interface_ip; ///< int variable representing interface_ip.
        std::string multicast_group; ///< int variable representing multicast_group.
        int port; ///< int variable representing port.
        bool non_blocking = true; ///< bool variable representing non_blocking.
        int receive_buffer_size = 16 * 1024 * 1024; // 16MB kernel buffer
    };

             /**
              * @brief Auto-generated description for MulticastReceiver.
              * @param config Parameter description.
              */
    explicit MulticastReceiver(const Config& config);
    /**
     * @brief Auto-generated description for ~MulticastReceiver.
     */
    ~MulticastReceiver();

         /**
          * @brief Auto-generated description for start.
          * @return bool value.
          */
    bool start();
         /**
          * @brief Auto-generated description for stop.
          */
    void stop();

    // Receive a single packet (Zero-Copy-ish: writes directly to provided buffer)
    // Returns bytes received, or -1 if empty/error
    int receive(uint8_t* buffer, size_t max_len);

private:
    Config config_; ///< Config variable representing config_.
    int sock_fd_{-1}; ///< int variable representing sock_fd_.
    std::atomic<bool> running_{false}; ///< int variable representing running_.
    struct sockaddr_in addr_; ///< struct sockaddr_in variable representing addr_.
};

} // namespace ultra::network
