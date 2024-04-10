/**
 * @file RandomDispatchEngineStrategy.hpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#pragma once

/* external libraries */
#include <vector>
#include <random>
/* internal libraries */
#include "simulator/strategies/DispatchEngineStrategy.hpp"
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"

class RandomDispatchEngineStrategy : public DispatchEngineStrategy {
 protected:
    static bool assigningAmbulance(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void dispatchingToHospital(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
    static void reallocating(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );

 public:
    static bool run(
        std::mt19937& rnd,
        std::vector<Ambulance>& ambulances,
        std::vector<Event>& events,
        const int eventIndex
    );
};
