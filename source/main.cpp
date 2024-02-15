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
    std::mt19937 rnd(0);

    Incidents incidents;
    incidents.loadFromFile("../../Data-Processing/data/enhanced/oslo/incidents.csv");

    Stations stations;
    stations.loadFromFile("../../Data-Processing/data/enhanced/oslo/depots.csv");

    ODMatrix odMatrix;
    odMatrix.loadFromFile("../../Data-Processing/data/oslo/od_matrix.txt");

    AmbulanceAllocator ambulanceAllocator(stations);
    ambulanceAllocator.allocate(std::vector<int>{2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1});

    MonteCarloSimulator monteCarloSim(rnd, incidents, 2019, 2, 7, true, 4);

    std::vector<Event> events = monteCarloSim.generateEvents();

    Simulator simulator(
        rnd,
        incidents,
        stations,
        odMatrix,
        ambulanceAllocator,
        DispatchEngineStrategyType::RANDOM,
        events
    );

    simulator.run();

    simulator.printAverageEventPerformanceMetrics();

    return 0;
}
