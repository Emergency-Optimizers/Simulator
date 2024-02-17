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
#include "simulator/DispatchEngineStrategyType.hpp"
#include "simulator/Ambulance.hpp"
#include "simulator/Event.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"

class DispatchEngine {
 private:
    static void randomStrategy(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        Event& event,
        int eventIndex
    );

 public:
    static void dispatch(
        const DispatchEngineStrategyType strategy,
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        Event& event,
        int eventIndex
    );
};
