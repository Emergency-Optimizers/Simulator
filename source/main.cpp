/**
 * @file Main.cpp
 *
 * @copyright Copyright (c) 2024 Emergency-Optimizers
 */

#include <iostream>
#include <chrono>
/* internal libraries */
#include "simulator/Incidents.hpp"
#include "simulator/Stations.hpp"
#include "simulator/ODMatrix.hpp"
#include "simulator/AmbulanceAllocator.hpp"
#include "simulator/Simulator.hpp"
#include "simulator/MonteCarloSimulator.hpp"
#include "genetic-algorithm/Individual.hpp"
#include "genetic-algorithm/Population.hpp"

int main() {
    std::mt19937 rnd(0);

    Incidents incidents("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    Stations stations("../../Data-Processing/data/enhanced/oslo/depots.csv");
    ODMatrix odMatrix("../../Data-Processing/data/oslo/od_matrix.txt");

    int populationSize = 10;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 10;
    bool saveEventsToCSV = true;

    std::cout << "Starting GA..." << std::endl;
    Population population(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, mutationProbability, saveEventsToCSV);

    // run the genetic algorithm for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    population.evolve(generations);
    auto end = std::chrono::high_resolution_clock::now();

    // find and print the fittest individual after the final generation
    Individual fittest = population.findFittest();
    std::cout << "Fittest Individual After Evolution: " << std::endl;
    std::cout << "Valid: " << fittest.isValid() << std::endl;

    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "GA completed in " << duration << " milliseconds." << std::endl;
    fittest.printChromosome();


    return 0;
}
