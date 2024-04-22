/**
 * @file Programs.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <filesystem>
/* internal libraries */
#include "heuristics/Programs.hpp"
#include "Utils.hpp"
#include "heuristics/PopulationGA.hpp"
#include "heuristics/PopulationNSGA2.hpp"
#include "heuristics/PopulationMA.hpp"
#include "heuristics/PopulationMemeticNSGA2.hpp"
#include "Constants.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"

void runSimulatorOnce(std::vector<Event> events) {
    // set allocation
    std::vector<std::vector<int>> allocations;
    allocations.push_back({2, 4, 2, 2, 2, 4, 2, 3, 3, 3, 3, 5, 4, 3, 3});

    if (Settings::get<bool>("SIMULATE_DAY_SHIFT")) {
        int allocationSize = static_cast<int>(allocations.size());
        for (int allocationIndex = 0; allocationIndex < allocationSize; allocationIndex++) {
            allocations[allocationIndex].push_back(0);
            allocations[allocationIndex].push_back(0);
            allocations[allocationIndex].push_back(0);
            allocations[allocationIndex].push_back(0);
        }
    }

    AmbulanceAllocator ambulanceAllocator;
    ambulanceAllocator.allocate(
        events,
        allocations,
        Settings::get<bool>("SIMULATE_DAY_SHIFT")
    );

    // simulate events
    Simulator simulator(
        ambulanceAllocator,
        Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
        events
    );
    std::vector<Event> simulatedEvents;
    simulatedEvents = simulator.run();

    // write events to file
    writeEvents(Settings::get<std::string>("UNIQUE_RUN_ID") + "_NONE", simulatedEvents);

    // print metrics
    double avgResponseTimeAUrban = averageResponseTime(simulatedEvents, "A", true);
    double avgResponseTimeANonurban = averageResponseTime(simulatedEvents, "A", false);
    double avgResponseTimeHUrban = averageResponseTime(simulatedEvents, "H", true);
    double avgResponseTimeHNonurban = averageResponseTime(simulatedEvents, "H", false);
    double avgResponseTimeV1Urban = averageResponseTime(simulatedEvents, "V1", true);
    double avgResponseTimeV1Nonurban = averageResponseTime(simulatedEvents, "V1", false);

    printAmbulanceWorkload(ambulanceAllocator.ambulances);

    std::cout
        << "\nGoal:" << std::endl
        << "\t A, urban: <12 min" << std::endl
        << "\t A, non-urban: <25 min" << std::endl
        << "\t H, urban: <30 min" << std::endl
        << "\t H, non-urban: <40 min" << std::endl
        << std::endl
        << "Avg. response time (A, urban): \t\t" << avgResponseTimeAUrban << "s (" << avgResponseTimeAUrban / 60 << "m)" << std::endl
        << "Avg. response time (A, non-urban): \t" << avgResponseTimeANonurban << "s (" << avgResponseTimeANonurban / 60 << "m)" << std::endl
        << "Avg. response time (H, urban): \t\t" << avgResponseTimeHUrban << "s (" << avgResponseTimeHUrban / 60 << "m)" << std::endl
        << "Avg. response time (H, non-urban): \t" << avgResponseTimeHNonurban << "s (" << avgResponseTimeHNonurban / 60 << "m)" << std::endl
        << "Avg. response time (V1, urban): \t" << avgResponseTimeV1Urban << "s (" << avgResponseTimeV1Urban / 60 << "m)" << std::endl
        << "Avg. response time (V1, non-urban): \t" << avgResponseTimeV1Nonurban << "s (" << avgResponseTimeV1Nonurban / 60 << "m)" << std::endl
        << "Percentage violations: \t\t\t" << responseTimeViolations(simulatedEvents) * 100 << "%" << std::endl;
}

void runGeneticAlgorithm(const std::vector<Event>& events) {
    PopulationGA population(events);
    population.evolve();
}

void runNSGA2(const std::vector<Event>& events) {
    PopulationNSGA2 population(events);
    population.evolve();
}

void runMemeticAlgorithm(const std::vector<Event>& events) {
    PopulationMA population(events);
    population.evolve();
}

void runMemeticNSGA2(const std::vector<Event>& events) {
    PopulationMemeticNSGA2 population(events);
    population.evolve();
}
