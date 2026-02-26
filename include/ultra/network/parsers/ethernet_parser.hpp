#pragma once
#include "../../core/compiler.hpp"
#include "../../core/types.hpp"
#include <cstring>

#if defined(__APPLE__)
    #include <net/ethernet.h>
    #include <netinet/if_ether.h>
#else
    #include <netinet/ether.h>
#endif

#include <netinet/ip.h>
#include <netinet/udp.h>

namespace ultra::network {

#pragma pack(push, 1)

struct EthernetHeader {
    uint8_t dst_mac[6]; ///< int[6] variable representing dst_mac.
    uint8_t src_mac[6]; ///< int[6] variable representing src_mac.
    uint16_t ethertype; ///< int variable representing ethertype.
};

struct IPv4Header {
    uint8_t  version_ihl; ///< int variable representing version_ihl.
    uint8_t  tos; ///< int variable representing tos.
    uint16_t total_length; ///< int variable representing total_length.
    uint16_t id; ///< int variable representing id.
    uint16_t flags_offset; ///< int variable representing flags_offset.
    uint8_t  ttl; ///< int variable representing ttl.
    uint8_t  protocol; ///< int variable representing protocol.
    uint16_t checksum; ///< int variable representing checksum.
    uint32_t src_ip; ///< int variable representing src_ip.
    uint32_t dst_ip; ///< int variable representing dst_ip.
};

struct UDPHeader {
    uint16_t src_port; ///< int variable representing src_port.
    uint16_t dst_port; ///< int variable representing dst_port.
    uint16_t length; ///< int variable representing length.
    uint16_t checksum; ///< int variable representing checksum.
};

#pragma pack(pop)

/**
 * Zero-copy Ethernet/IP/UDP parser
 * Parses in-place, no allocations
 */
class EthernetParser {
public:
    struct ParsedPacket {
        const uint8_t* payload; ///< const int * variable representing payload.
        uint16_t payload_len; ///< int variable representing payload_len.
        uint32_t src_ip; ///< int variable representing src_ip.
        uint32_t dst_ip; ///< int variable representing dst_ip.
        uint16_t src_port; ///< int variable representing src_port.
        uint16_t dst_port; ///< int variable representing dst_port.
        Timestamp timestamp_ns; ///< int variable representing timestamp_ns.
        bool valid; ///< bool variable representing valid.
    };
    
                                            /**
                                             * @brief Auto-generated description for parse.
                                             * @param packet Parameter description.
                                             * @param packet_len Parameter description.
                                             * @param timestamp_ns Parameter description.
                                             * @return ParsedPacket value.
                                             */
    ULTRA_ALWAYS_INLINE static ParsedPacket parse(
        const uint8_t* packet, 
        size_t packet_len,
        Timestamp timestamp_ns
    ) noexcept;
    
                                        /**
                                         * @brief Auto-generated description for ntohs_fast.
                                         * @param n Parameter description.
                                         * @return int value.
                                         */
    ULTRA_ALWAYS_INLINE static uint16_t ntohs_fast(uint16_t n) noexcept {
        return __builtin_bswap16(n);
    }
    
                                        /**
                                         * @brief Auto-generated description for ntohl_fast.
                                         * @param n Parameter description.
                                         * @return int value.
                                         */
    ULTRA_ALWAYS_INLINE static uint32_t ntohl_fast(uint32_t n) noexcept {
        return __builtin_bswap32(n);
    }
};

} // namespace ultra::network
