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
#include "heuristics/PopulationMemeticNSGA2.hpp"
#include "heuristics/HeuristicType.hpp"
#include "heuristics/Programs.hpp"

int main() {
    auto start = std::chrono::high_resolution_clock::now();

    std::locale::global(std::locale("en_US.utf8"));

    Settings::LoadSettings();
    Traffic::getInstance();
    Stations::getInstance();
    Incidents::getInstance();
    ODMatrix::getInstance();

    std::cout << std::endl;

    // generate events
    MonteCarloSimulator monteCarloSim;
    std::vector<Event> events = monteCarloSim.generateEvents();

    std::cout << std::endl;

    // run heuristic
    const HeuristicType heuristic = Settings::get<HeuristicType>("HEURISTIC");
    switch (heuristic) {
        case HeuristicType::NONE:
            runSimulatorOnce(events);

            break;
        case HeuristicType::GA:
            runGeneticAlgorithm(events);

            break;
        case HeuristicType::NSGA2:
            runNSGA2(events);

            break;
        case HeuristicType::MA:
            runMemeticAlgorithm(events);

            break;
        case HeuristicType::MEMETIC_NSGA2:
            runMemeticNSGA2(events);

            break;
        case HeuristicType::CUSTOM:
            runSimulationGridSearch(events);

            break;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "\n\nProgram took " << ((duration / 1000) / 60) << " minutes to complete." << std::endl;

    return 0;
}
