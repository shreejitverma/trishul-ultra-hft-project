#include "ultra/core/symbol_universe.hpp"
#include <iostream>
#include <cassert>

using namespace ultra;

int main() {
    std::cout << "[Test] Starting SymbolUniverse Test...\n";

    auto& universe = SymbolUniverse::instance();

    SymbolInfo aapl{1, "AAPL", 1, 1, -0.0002, 0.0003};
    SymbolInfo tsla{2, "TSLA", 1, 5, -0.0002, 0.0003};

    universe.add_symbol(aapl);
    universe.add_symbol(tsla);

    assert(universe.size() == 2);
    
    auto s1 = universe.get_symbol(1);
    assert(s1 != nullptr);
    std::cout << "  -> Found: " << s1->name << "\n";

    auto id = universe.get_id("TSLA");
    assert(id == 2);
    std::cout << "  -> ID: " << id << "\n";

    std::cout << "[Test] SymbolUniverse Test Passed.\n";
    return 0;
}
