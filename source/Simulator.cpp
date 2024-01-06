/**
 * @file Simulator.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2024-01-06
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* internal libraries */
#include "Simulator.hpp"

Simulator::Simulator(
    Incidents& incidents,
    Stations& stations,
    AmbulanceAllocator& ambulanceAllocator,
    const std::string& start,
    const std::string& end
) : incidents(incidents), stations(stations), ambulanceAllocator(ambulanceAllocator) {
    eventHandler.populate(incidents, start, end);
}
