/**
 * @file Simulator.hpp
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
#include "Incidents.hpp"
#include "Stations.hpp"
#include "AmbulanceAllocator.hpp"
#include "EventHandler.hpp"

class Simulator {
 private:
    Incidents& incidents;
    Stations& stations;
    AmbulanceAllocator& ambulanceAllocator;
    EventHandler eventHandler;

 public:
    Simulator(
        Incidents& incidents,
        Stations& stations,
        AmbulanceAllocator& ambulanceAllocator,
        const std::string& start,
        const std::string& end
    );
};
