/**
 * @file DispatchEngine.hpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

#pragma once

/* external libraries */
#include <vector>
/* internal libraries */
#include "DispatchEngineStrategy.hpp"
#include "Ambulance.hpp"
#include "Event.hpp"

class DispatchEngine {
 public:
    static void dispatch(const DispatchEngineStrategy strategy, std::vector<Ambulance>& ambulances, Event& event);
};
