#pragma once
#include "huge_page_allocator.hpp"
#include <vector>
#include <cassert>
#include <memory>

namespace ultra {

/**
 * Fixed-size object pool for ultra-low latency allocations.
 * - Uses HugePageAllocator for backing memory.
 * - O(1) allocate/deallocate.
 * - Cache-friendly (contiguous memory).
 */
template<typename T, size_t PoolSize>
class ObjectPool {
public:
    /**
     * @brief Auto-generated description for ObjectPool<T, PoolSize>.
     */
    ObjectPool() {
        // Allocate contiguous block
        memory_block_ = allocator_.allocate(PoolSize);
        
        // Initialize free list
        for (size_t i = 0; i < PoolSize; ++i) {
            free_indices_[i] = i;
        }
        free_count_ = PoolSize;
    }
    
    /**
     * @brief Auto-generated description for ~ObjectPool<T, PoolSize>.
     */
    ~ObjectPool() {
        if (memory_block_) {
            allocator_.deallocate(memory_block_, PoolSize);
        }
    }
    
    // Non-copyable/movable to keep it simple
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator=(const ObjectPool&) = delete;
    
    template<typename... Args>
       /**
        * @brief Auto-generated description for allocate.
        * @return T * value.
        */
    T* allocate(Args&&... args) noexcept {
        if (ULTRA_UNLIKELY(free_count_ == 0)) {
            return nullptr; // Pool exhausted
        }
        
        uint32_t index = free_indices_[--free_count_];
        T* ptr = &memory_block_[index];
        new (ptr) T(std::forward<Args>(args)...); // Construct in-place
        return ptr;
    }
    
         /**
          * @brief Auto-generated description for deallocate.
          * @param ptr Parameter description.
          */
    void deallocate(T* ptr) noexcept {
        // Calculate index from pointer
        size_t index = ptr - memory_block_;
        assert(index < PoolSize && "Pointer not from this pool");
        
        ptr->~T(); // Destruct
        free_indices_[free_count_++] = static_cast<uint32_t>(index);
    }
    
         /**
          * @brief Auto-generated description for clear.
          */
    void clear() noexcept {
        free_count_ = PoolSize;
        // Optimization: Don't destruct everything if T is trivial
    }

private:
    HugePageAllocator<T> allocator_; ///< HugePageAllocator<T> variable representing allocator_.
    T* memory_block_{nullptr}; ///< T * variable representing memory_block_.
    
    // Stack-based free list
    std::array<uint32_t, PoolSize> free_indices_; ///< int variable representing free_indices_.
    size_t free_count_{0}; ///< int variable representing free_count_.
};

} // namespace ultra
