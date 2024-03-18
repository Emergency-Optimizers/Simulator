/**
 * @file ClosestDispatchEngineStrategy.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategy.hpp"

class ClosestDispatchEngineStrategy : public DispatchEngineStrategy {
 protected:
    static void assigningAmbulance(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToHospital(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );

 public:
    static void run(
        std::mt19937& rng,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
};
