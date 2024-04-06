/**
 * @file DispatchEngine.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategyType.hpp"
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"

class DispatchEngine {
 public:
    static void dispatch(
        const DispatchEngineStrategyType strategy,
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
};
