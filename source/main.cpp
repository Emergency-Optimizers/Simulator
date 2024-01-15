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
#include "ODMatrix.hpp"
#include "Incidents.hpp"
#include "Stations.hpp"
#include "AmbulanceAllocator.hpp"
#include "Simulator.hpp"

/**
 * Main program.
 */
int main() {
    const unsigned seed = 0;

    ODMatrix odMatrix;
    odMatrix.loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");

    Stations stations;
    stations.loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");

    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");

    AmbulanceAllocator ambulanceAllocator(stations);
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19};
    ambulanceAllocator.allocate(v);

    Simulator simulator(seed, incidents, stations, odMatrix, ambulanceAllocator, DispatchEngineStrategy::RANDOM, "2016.11.29T00:00:00", "2016.11.29T23:59:59");

    simulator.run();

    return 0;
}
