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
    std::cout << "[1/3] Incidents loaded" << std::endl;
    Stations stations("../../Data-Processing/data/enhanced/oslo/depots.csv");
    std::cout << "[2/3] Stations loaded" << std::endl;
    ODMatrix odMatrix("../../Data-Processing/data/oslo/od_matrix.txt");
    std::cout << "[3/3] OD matrix loaded" << std::endl;

    
    int numObjectives = 7;
    int populationSize = 30;
    int numDepots = 19;
    int numAmbulances = 45;
    double mutationProbability = 0.05;
    int generations = 30;
    bool saveEventsToCSV = true;

    std::cout << "\nStarting NSGA..." << std::endl;
    Population population(rnd, incidents, stations, odMatrix, populationSize, numDepots, numAmbulances, numObjectives, mutationProbability, saveEventsToCSV);

    // run the NSGA for the specified number of generations
    auto start = std::chrono::high_resolution_clock::now();
    population.evolve(generations);
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    return 0;
}
