/**
 * @file main.cpp
 * @author Sindre Eiklid
 * @version 1.0
 * @date 2023-11-07
 *
 * @copyright Copyright (c) 2023 Sindre Eiklid
 */

/* external libraries */
#include <iostream>
/* internal libraries */
#include "simulator/ODMatrix.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/MonteCarloSimulator.hpp"

/**
 * Main program.
 */
int main() {
    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");

    MonteCarloSimulator sim(incidents, 2, 7, 2);

    return 0;
}
