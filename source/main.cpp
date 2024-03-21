/**
 * @file Main.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

/* external libraries */
#include <iostream>
#include <chrono>
/* internal libraries */
#include "Settings.hpp"
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/Traffic.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/MonteCarloSimulator.hpp"
#include "genetic-algorithm/Individual.hpp"
#include "genetic-algorithm/Population.hpp"

int main() {
    std::mt19937 rnd(0);

    Settings::LoadSettings();
    Incidents::getInstance();
    Stations::getInstance();
    ODMatrix::getInstance();
    Traffic::getInstance();

    bool saveEventsToCSV = true;

    std::cout << "Starting GA..." << std::endl;
    Population population(
        rnd,
        Settings::get<int>("POPULATION_SIZE"),
        Settings::get<float>("MUTATION_PROBABILITY"),
        Settings::get<bool>("SIMULATE_DAY_SHIFT"),
        saveEventsToCSV
    );

    // run the genetic algorithm for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    population.evolve(Settings::get<int>("GENERATION_SIZE"));
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // find and print the fittest individual after the final generation
    std::cout << "\n[--- GA completed in " << duration / 1000 << " seconds ---]" << std::endl;
    std::cout << "Generations: " << Settings::get<int>("GENERATION_SIZE") << ", population size: " << Settings::get<int>("POPULATION_SIZE") << std::endl;
    Individual fittest = population.findFittest();
    std::cout << "Fittest Individual: " << fittest.getFitness();
    std::cout << (fittest.isValid() ? " [valid]\n" : " [invalid]\n")  << std::endl;

    fittest.printChromosome();

    return 0;
}
