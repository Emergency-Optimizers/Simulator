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

int main() {
    std::locale::global(std::locale("en_US.utf8"));
    std::mt19937 rnd(0);

    Settings::LoadSettings();
    Incidents::getInstance();
    Stations::getInstance();
    ODMatrix::getInstance();
    Traffic::getInstance();

    std::cout << "Starting GA..." << std::endl;
    PopulationGA population(
        rnd,
        Settings::get<int>("POPULATION_SIZE"),
        Settings::get<float>("MUTATION_PROBABILITY"),
        Settings::get<bool>("SIMULATE_DAY_SHIFT")
    );

    // run the genetic algorithm for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    population.evolve(Settings::get<int>("GENERATION_SIZE"));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // find and print the fittest individual after the final generation
    std::cout << "\n[--- GA completed in " << duration / 1000 << " seconds ---]" << std::endl;
    std::cout << "Generations: " << Settings::get<int>("GENERATION_SIZE") << ", population size: "
        << Settings::get<int>("POPULATION_SIZE") << std::endl;
    IndividualGA fittest = population.findFittest();
    std::cout << "Fittest IndividualGA: " << fittest.getFitness();
    std::cout << (fittest.isValid() ? " [valid]\n" : " [invalid]\n")  << std::endl;

    fittest.printChromosome();

    return 0;
}
