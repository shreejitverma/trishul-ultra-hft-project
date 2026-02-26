#pragma once
#include <atomic>
#include <cstdint>
#include <vector>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include "../../core/compiler.hpp"

namespace ultra::network {

/**
 * Simulated DMA Ring Buffer (Fig 1)
 * Mimics the shared memory structure used by VFIO-PCI cards (e.g., Solarflare, Mellanox).
 * In production, this memory is mapped from the NIC's BAR or pinned RAM.
 */
class DMARingBuffer {
public:
    static constexpr size_t SLOT_SIZE = 2048; // Standard MTU + padding
    static constexpr size_t RING_SIZE = 1024; // Power of 2
    static constexpr size_t BUFFER_SIZE = SLOT_SIZE * RING_SIZE; ///< const int variable representing BUFFER_SIZE.

    struct ControlBlock {
        alignas(64) std::atomic<uint32_t> head{0}; // Written by NIC (Producer)
        alignas(64) std::atomic<uint32_t> tail{0}; // Written by SW (Consumer)
    };

    /**
     * @brief Auto-generated description for DMARingBuffer.
     */
    DMARingBuffer() {
        // Allocate simulated DMA memory
        // In real code: mmap /dev/vfio/...
        void* ptr = mmap(nullptr, BUFFER_SIZE + sizeof(ControlBlock), 
                         PROT_READ | PROT_WRITE, 
                         MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (ptr == MAP_FAILED) {
            std::cerr << "Failed to allocate DMA buffer\n";
            abort();
        }
        
        control_ = new (ptr) ControlBlock();
        buffer_base_ = reinterpret_cast<uint8_t*>(ptr) + sizeof(ControlBlock);
    }

    /**
     * @brief Auto-generated description for ~DMARingBuffer.
     */
    ~DMARingBuffer() {
        // munmap ...
    }

    // --- Consumer API (Poller Thread) ---
    
    // Check if new data is available
    ULTRA_HOT inline bool has_data() const {
        return control_->tail.load(std::memory_order_acquire) != 
               control_->head.load(std::memory_order_acquire);
    }

    // Get pointer to current packet
    ULTRA_HOT inline const uint8_t* peek() const {
        uint32_t idx = control_->tail.load(std::memory_order_relaxed) & (RING_SIZE - 1);
        return buffer_base_ + (idx * SLOT_SIZE);
    }

    // Mark current packet as processed
    ULTRA_HOT inline void advance() {
        // Increment tail
        control_->tail.fetch_add(1, std::memory_order_release);
    }

    // --- Producer API (Simulating FPGA DMA) ---
    // Only used for testing/simulation
    void write_packet(const uint8_t* data, size_t len) {
        uint32_t head = control_->head.load(std::memory_order_relaxed);
        uint32_t idx = head & (RING_SIZE - 1);
        uint8_t* dest = buffer_base_ + (idx * SLOT_SIZE);
        
        // Simple memcpy simulating DMA write
        size_t copy_len = (len > SLOT_SIZE) ? SLOT_SIZE : len;
        std::memcpy(dest, data, copy_len);
        
        // Update head (commit)
        control_->head.fetch_add(1, std::memory_order_release);
    }

private:
    ControlBlock* control_; ///< ControlBlock * variable representing control_.
    uint8_t* buffer_base_; ///< int * variable representing buffer_base_.
};

} // namespace ultra::network
