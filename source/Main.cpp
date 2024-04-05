/**
 * @file Main.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
#include <chrono>
/* internal libraries */
#include "file-reader/Settings.hpp"
#include "file-reader/Incidents.hpp"
#include "file-reader/Stations.hpp"
#include "file-reader/ODMatrix.hpp"
#include "file-reader/Traffic.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/MonteCarloSimulator.hpp"
#include "heuristics/IndividualGA.hpp"
#include "heuristics/PopulationGA.hpp"
#include "heuristics/IndividualNSGA.hpp"
#include "heuristics/PopulationNSGA.hpp"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    std::locale::global(std::locale("en_US.utf8"));

    Settings::LoadSettings();
    Traffic::getInstance();
    Stations::getInstance();
    Incidents::getInstance();
    ODMatrix::getInstance();

    std::mt19937 rnd(Settings::get<int>("SEED"));

    std::string heuristic = Settings::get<std::string>("HEURISTIC");
    if (heuristic == "GA") {
        PopulationGA population(
            rnd,
            Settings::get<int>("POPULATION_SIZE"),
            Settings::get<float>("MUTATION_PROBABILITY"),
            Settings::get<float>("CROSSOVER_PROBABILITY"),
            Settings::get<bool>("SIMULATE_DAY_SHIFT"),
            Settings::get<int>("NUM_TIME_SEGMENTS")
        );
        population.evolve(Settings::get<int>("GENERATION_SIZE"));
    } else if (heuristic == "NSGA") {
        bool useFronts = Settings::get<bool>("USE_NSGA_FRONTS");
        PopulationNSGA population(
            rnd,
            useFronts,
            Settings::get<std::vector<float>>("NSGA_WEIGHTS"),
            Settings::get<int>("POPULATION_SIZE"),
            Settings::get<float>("MUTATION_PROBABILITY"),
            Settings::get<float>("CROSSOVER_PROBABILITY"),
            Settings::get<bool>("SIMULATE_DAY_SHIFT"),
            Settings::get<int>("NUM_TIME_SEGMENTS")
        );
        population.evolve(Settings::get<int>("GENERATION_SIZE"));
    } else if (heuristic == "NONE") {
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

        // set allocation
        std::vector<std::vector<int>> allocations;
        allocations.push_back({2, 4, 2, 2, 2, 4, 2, 3, 3, 3, 3, 5, 4, 3, 3});

        if (Settings::get<bool>("SIMULATE_DAY_SHIFT")) {
            allocations[0].push_back(0);
            allocations[0].push_back(0);
            allocations[0].push_back(0);
            allocations[0].push_back(0);
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
        simulator.run(true);

        // print metrics
        double avgResponseTimeAUrban = simulator.averageResponseTime("A", true);
        double avgResponseTimeANonurban = simulator.averageResponseTime("A", false);
        double avgResponseTimeHUrban = simulator.averageResponseTime("H", true);
        double avgResponseTimeHNonurban = simulator.averageResponseTime("H", false);
        double avgResponseTimeV1Urban = simulator.averageResponseTime("V1", true);
        double avgResponseTimeV1Nonurban = simulator.averageResponseTime("V1", false);

        std::cout
            << "Goal:" << std::endl
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
            << "Total violations: \t\t\t" << simulator.responseTimeViolations() << std::endl;
    } else {
        std::cout << "Unknown heuristic given." << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "\n\nProgram took " << ((duration / 1000) / 60) << " minutes to complete." << std::endl;

    return 0;
}