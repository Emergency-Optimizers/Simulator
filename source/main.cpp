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
    std::mt19937 rnd(11);

    Settings::LoadSettings();
    Traffic::getInstance();
    Stations::getInstance();
    Incidents::getInstance();
    ODMatrix::getInstance();

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
            Settings::get<bool>("SIMULATE_DAY_SHIFT")
        );
        population.evolve(Settings::get<int>("GENERATION_SIZE"));
    } else {
        std::cout << "Unknown heuristic given." << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "\n\nProgram took " << ((duration / 1000) / 60) << " minutes to complete." << std::endl;

    return 0;
}
