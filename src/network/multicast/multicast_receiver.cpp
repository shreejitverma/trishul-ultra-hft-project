#include "ultra/network/multicast_receiver.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>

namespace ultra::network {

                   /**
                    * @brief Auto-generated description for MulticastReceiver.
                    * @param config Parameter description.
                    */
MulticastReceiver::MulticastReceiver(const Config& config) : config_(config) {}

                   /**
                    * @brief Auto-generated description for ~MulticastReceiver.
                    */
MulticastReceiver::~MulticastReceiver() {
    stop();
}

                        /**
                         * @brief Auto-generated description for start.
                         * @return bool value.
                         */
bool MulticastReceiver::start() {
    sock_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd_ < 0) {
        perror("socket");
        return false;
    }

    // 1. Allow multiple sockets to use the same PORT number
    int reuse = 1;
    if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEADDR)");
        return false;
    }
#ifdef SO_REUSEPORT
    if (setsockopt(sock_fd_, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt(SO_REUSEPORT)");
    }
#endif

    // 2. Increase Kernel Receive Buffer (Crucial for HFT to prevent drops during bursts)
    if (setsockopt(sock_fd_, SOL_SOCKET, SO_RCVBUF, &config_.receive_buffer_size, sizeof(config_.receive_buffer_size)) < 0) {
        perror("setsockopt(SO_RCVBUF)");
        std::cerr << "Warning: Could not set large socket buffer. Check sysctl wmem_max." << std::endl;
    }

    // 3. Bind to the Port
    memset(&addr_, 0, sizeof(addr_));
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(config_.port);
    addr_.sin_addr.s_addr = htonl(INADDR_ANY); // Listen on all interfaces

    if (bind(sock_fd_, (struct sockaddr*)&addr_, sizeof(addr_)) < 0) {
        perror("bind");
        return false;
    }

    // 4. Join Multicast Group
    struct ip_mreq group;
    group.imr_multiaddr.s_addr = inet_addr(config_.multicast_group.c_str());
    group.imr_interface.s_addr = inet_addr(config_.interface_ip.c_str()); 

    if (setsockopt(sock_fd_, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group, sizeof(group)) < 0) {
        perror("setsockopt(IP_ADD_MEMBERSHIP)");
        return false;
    }

    // 5. Set Non-Blocking if requested
    if (config_.non_blocking) {
        int flags = fcntl(sock_fd_, F_GETFL, 0);
        if (fcntl(sock_fd_, F_SETFL, flags | O_NONBLOCK) < 0) {
            perror("fcntl(O_NONBLOCK)");
            return false;
        }
    }

    running_ = true;
    std::cout << "UDP Receiver started on " << config_.multicast_group << ":" << config_.port << std::endl;
    return true;
}

                        /**
                         * @brief Auto-generated description for stop.
                         */
void MulticastReceiver::stop() {
    running_ = false;
    if (sock_fd_ >= 0) {
        close(sock_fd_);
        sock_fd_ = -1;
    }
}

                                 /**
                                  * @brief Auto-generated description for receive.
                                  * @param buffer Parameter description.
                                  * @param max_len Parameter description.
                                  * @return int value.
                                  */
ULTRA_HOT int MulticastReceiver::receive(uint8_t* buffer, size_t max_len) {
    if (!running_) return -1;

    ssize_t n = recv(sock_fd_, buffer, max_len, 0);
    if (n < 0) {
        // In non-blocking mode, EAGAIN/EWOULDBLOCK means no data
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // No data
        }
        perror("recv");
        return -1; // Error
    }
    return static_cast<int>(n);
}

} // namespace ultra::network
