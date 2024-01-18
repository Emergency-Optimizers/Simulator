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
    std::vector<int> v = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    ambulanceAllocator.allocate(v);

    Simulator simulator(
        seed,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator,
        DispatchEngineStrategy::RANDOM,
        "2018.01.01T00:00:00",
        "2018.01.02T00:00:00"
    );

    simulator.run();

    simulator.printAverageEventPerformanceMetrics();

    return 0;
}
