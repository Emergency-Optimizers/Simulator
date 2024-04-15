/**
 * @file Main.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
#include <chrono>
#include <map>
#include <vector>
#include <string>
/* internal libraries */
#include "Utils.hpp"
#include "file-reader/Settings.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Traffic.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/MonteCarloSimulator.hpp"
#include "heuristics/Individual.hpp"
#include "heuristics/PopulationGA.hpp"
#include "heuristics/PopulationNSGA2.hpp"
#include "heuristics/PopulationMA.hpp"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    std::locale::global(std::locale("en_US.utf8"));

    Settings::LoadSettings();
    Traffic::getInstance();
    Stations::getInstance();
    Incidents::getInstance();
    ODMatrix::getInstance();

    std::mt19937 rnd(Settings::get<int>("SEED"));

    // generate events
    MonteCarloSimulator monteCarloSim(
        rnd,
        Settings::get<int>("SIMULATE_YEAR"),
        Settings::get<int>("SIMULATE_MONTH"),
        Settings::get<int>("SIMULATE_DAY"),
        Settings::get<bool>("SIMULATE_DAY_SHIFT"),
        Settings::get<int>("SIMULATION_GENERATION_WINDOW_SIZE")
    );
    std::vector<Event> events = monteCarloSim.generateEvents();

    // run heuristic
    std::string heuristic = Settings::get<std::string>("HEURISTIC");
    if (heuristic == "GA") {
        PopulationGA population(
            rnd,
            events,
            Settings::get<bool>("SIMULATE_DAY_SHIFT"),
            Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
            Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY"),
            Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT"),
            Settings::get<int>("POPULATION_SIZE"),
            Settings::get<float>("MUTATION_PROBABILITY"),
            Settings::get<float>("CROSSOVER_PROBABILITY"),
            Settings::get<int>("NUM_TIME_SEGMENTS")
        );
        population.evolve(Settings::get<int>("GENERATION_SIZE"));
    } else if (heuristic == "NSGA") {
        PopulationNSGA2 population(
            rnd,
            events,
            Settings::get<bool>("SIMULATE_DAY_SHIFT"),
            Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
            Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY"),
            Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT"),
            Settings::get<int>("POPULATION_SIZE"),
            Settings::get<float>("MUTATION_PROBABILITY"),
            Settings::get<float>("CROSSOVER_PROBABILITY"),
            Settings::get<int>("NUM_TIME_SEGMENTS")
        );
        population.evolve(Settings::get<int>("GENERATION_SIZE"));
    } else if (heuristic == "MA") {
        PopulationMA population(
            rnd,
            events,
            Settings::get<bool>("SIMULATE_DAY_SHIFT"),
            Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
            Settings::get<int>("TOTAL_AMBULANCES_DURING_DAY"),
            Settings::get<int>("TOTAL_AMBULANCES_DURING_NIGHT"),
            Settings::get<int>("POPULATION_SIZE"),
            Settings::get<float>("MUTATION_PROBABILITY"),
            Settings::get<float>("CROSSOVER_PROBABILITY"),
            Settings::get<int>("NUM_TIME_SEGMENTS")
        );
        population.evolve(Settings::get<int>("GENERATION_SIZE"));
    } else if (heuristic == "NONE") {
        // set allocation
        std::vector<std::vector<int>> allocations;
        allocations.push_back({2, 4, 2, 2, 2, 4, 2, 3, 3, 3, 3, 5, 4, 3, 3});

        if (Settings::get<bool>("SIMULATE_DAY_SHIFT")) {
            for (int allocationIndex = 0; allocationIndex < allocations.size(); allocationIndex++) {
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
            rnd,
            ambulanceAllocator,
            Settings::get<DispatchEngineStrategyType>("DISPATCH_STRATEGY"),
            events
        );
        std::vector<Event> simulatedEvents = simulator.run();

        // write metrics to file
        writeMetrics(Settings::get<std::string>("UNIQUE_RUN_ID"), simulatedEvents);

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
    } else {
        std::cout << "Unknown heuristic given." << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "\n\nProgram took " << ((duration / 1000) / 60) << " minutes to complete." << std::endl;

    return 0;
}
