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
#include "metaheuristics/nsga2/Individual.hpp"
#include "metaheuristics/nsga2/Population.hpp"

int main() {
    std::mt19937 rnd(0);

    Incidents incidents("../../Data-Processing/data/enhanced/oslo/incidents.csv");
    Stations stations("../../Data-Processing/data/enhanced/oslo/depots.csv");
    ODMatrix odMatrix("../../Data-Processing/data/oslo/od_matrix.txt");
    
    int numObjectives = 3;
    int populationSize = 10;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 3;
    bool saveEventsToCSV = true;

    std::cout << "Starting NSGA..." << std::endl;
    Population population(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, numObjectives, mutationProbability, saveEventsToCSV);

    // run the genetic algorithm for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    population.evolve(generations);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Got out! " << std::endl;

    return 0;
}
