#pragma once
#include "ultra/execution/oms/oms_core.hpp"

namespace ultra::execution {

using VenueId = uint32_t;

class SmartOrderRouter {
public:
    void route(const OrderState& order);
};

} // namespace ultra::execution
