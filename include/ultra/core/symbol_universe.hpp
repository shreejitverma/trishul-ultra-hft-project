#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include "types.hpp"

namespace ultra {

struct SymbolInfo {
    SymbolId id; ///< int variable representing id.
    std::string name; ///< int variable representing name.
    uint32_t lot_size; ///< int variable representing lot_size.
    Price tick_size; ///< int variable representing tick_size.
    double maker_fee; ///< double variable representing maker_fee.
    double taker_fee; ///< double variable representing taker_fee.
    bool use_fpga_execution{false}; // New: Routing Flag
};

/**
 * Manages the universe of tradable symbols.
 * Provides O(1) lookup by ID.
 */
class SymbolUniverse {
public:
    static SymbolUniverse& instance() {
        static SymbolUniverse instance;
        return instance;
    }

         /**
          * @brief Auto-generated description for add_symbol.
          * @param info Parameter description.
          */
    void add_symbol(const SymbolInfo& info) {
        if (info.id >= symbols_.size()) {
            symbols_.resize(info.id + 1);
        }
        symbols_[info.id] = info;
        name_to_id_[info.name] = info.id;
    }

                      /**
                       * @brief Auto-generated description for get_symbol.
                       * @param id Parameter description.
                       * @return const SymbolInfo * value.
                       */
    const SymbolInfo* get_symbol(SymbolId id) const {
        if (id < symbols_.size()) return &symbols_[id];
        return nullptr;
    }

             /**
              * @brief Auto-generated description for get_id.
              * @param name Parameter description.
              * @return int value.
              */
    SymbolId get_id(const std::string& name) const {
        auto it = name_to_id_.find(name);
        if (it != name_to_id_.end()) return it->second;
        return INVALID_SYMBOL;
    }

           /**
            * @brief Auto-generated description for size.
            * @return int value.
            */
    size_t size() const { return name_to_id_.size(); }

private:
    /**
     * @brief Auto-generated description for SymbolUniverse.
     */
    SymbolUniverse() = default;
    
    std::vector<SymbolInfo> symbols_; ///< int variable representing symbols_.
    std::unordered_map<std::string, SymbolId> name_to_id_; ///< int variable representing name_to_id_.
};

} // namespace ultra
