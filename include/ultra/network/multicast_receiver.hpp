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
        std::string interface_ip;
        std::string multicast_group;
        int port;
        bool non_blocking = true;
        int receive_buffer_size = 16 * 1024 * 1024; // 16MB kernel buffer
    };

    explicit MulticastReceiver(const Config& config);
    ~MulticastReceiver();

    bool start();
    void stop();

    // Receive a single packet (Zero-Copy-ish: writes directly to provided buffer)
    // Returns bytes received, or -1 if empty/error
    int receive(uint8_t* buffer, size_t max_len);

private:
    Config config_;
    int sock_fd_{-1};
    std::atomic<bool> running_{false};
    struct sockaddr_in addr_;
};

} // namespace ultra::network
