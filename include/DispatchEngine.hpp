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
#include <random>
/* internal libraries */
#include "DispatchEngineStrategy.hpp"
#include "Ambulance.hpp"
#include "Event.hpp"
#include "Incidents.hpp"
#include "Stations.hpp"
#include "ODMatrix.hpp"

class DispatchEngine {
 private:
    static void randomStrategy(
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        Event& event
    );

 public:
    static void dispatch(
        const DispatchEngineStrategy strategy,
        std::mt19937& rng,
        Incidents& incidents,
        Stations& stations,
        ODMatrix& odMatrix,
        std::vector<Ambulance>& ambulances,
        Event& event
    );
};
